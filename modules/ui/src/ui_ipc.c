#include "ui_ipc.h"

#include <assert.h>
#include <errno.h>
#include <memory.h>
#include <printf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data/hashmap.h"
#include "data/hashmap_helpers.h"
#include "log.h"

// Remind you this is on browser process only

extern s32 pilot_ipc_entry_c_create(struct map_T *map, const c8 *name,
                                    void (*callback)(void *data),
                                    struct args_T *in_args,
                                    struct args_T *out_args) {
  assert(in_args && out_args && callback && name);

  if (gen_map_find(map, name)) {
    WARN("entry already exists");
    return 2;
  }

  struct entry_c_T *e_c = malloc(sizeof(struct entry_c_T));
  if (!e_c)
    goto err_e;

  enum ARG_TYPE *in_cpy = malloc(sizeof(enum ARG_TYPE) * in_args->n_args);
  if (!in_cpy)
    goto err_in;

  memcpy(in_cpy, in_args->args, sizeof(*in_cpy) * in_args->n_args);

  enum ARG_TYPE *out_cpy = malloc(sizeof(enum ARG_TYPE) * out_args->n_args);
  if (!out_cpy)
    goto err_out;

  memcpy(out_cpy, out_args->args, sizeof(*out_cpy) * out_args->n_args);

  e_c->e.in_args = (struct args_T){.args = in_cpy, .n_args = in_args->n_args};
  e_c->e.out_args =
      (struct args_T){.args = out_cpy, .n_args = out_args->n_args};
  e_c->callback = callback;

  gen_map_insert(map, name, e_c);
  return 0;

err_out:
  free(in_cpy);
err_in:
  free(e_c);
err_e:
  WARN("failed to allocate memory for new entry");
  return 1;
};

static u32 pilot_ipc_type_size(enum ARG_TYPE type);

static u32 pilot_ipc_type_size(enum ARG_TYPE type) {
  u32 res;
  switch (type) {
  case U32:
    res = sizeof(u32);
    break;
  case S32:
    res = sizeof(s32);
    break;
  }
  return res;
};

static u32 pilot_ipc_args_get_size_(struct args_T *args);

static u32 pilot_ipc_args_get_size_(struct args_T *args) {
  u32 res = 0;
  u32 i;
  for (i = 0; i < args->n_args; i++) {
    res += pilot_ipc_type_size(args->args[i]);
  }
  return res;
};

extern u32 pilot_ipc_dicc_get_size(struct map_T *map, u32 *n_entries) {

  u32 res = 0;
  res += sizeof(u32); // TOTAL NUMBER OF ENTRIES

  struct map_it_T it;
  gen_map_it_set_begin(map, &it);
  *n_entries = 0;
  while (it.current) {
    struct entry_c_T *entry = it.current->value;
    res += sizeof(enum ARG_TYPE) * entry->e.out_args.n_args;
    res += sizeof(enum ARG_TYPE) * entry->e.in_args.n_args;
    res += sizeof(u32) * 2; // n_args * 2 (in plus out)
    res += (strlen((c8 *)it.current->key) + 1) *
           sizeof(c8); // plus the size of the name +
                       // null terminator

    *n_entries += 1;
    gen_map_it_get_next(map, &it);
  };

  return res;
};

extern void temp_print_buffer(void *stream, const u32 size) {
  u32 i = 0;
  void *cpy = stream;
  printf("printing...:\n");

  u32 *n = cpy;
  INFO("entries n.: %d\n", *n);
  cpy += sizeof(u32);

  int j;
  for (j = 0; j < *n; j++) {
    c8 *first_n = cpy;
    INFO("name: %s", first_n);
    cpy += strlen(first_n) + 1;

    u32 *n_ins = cpy;
    INFO("ins n: %d", *n_ins);
    cpy += sizeof(u32);

    for (i = 0; i < *n_ins; i++) {
      enum ARG_TYPE *at = cpy;
      INFO("arg type[%d]: %d", i, *at);
      cpy += sizeof(enum ARG_TYPE);
    };

    u32 *n_outs = cpy;
    INFO("outs n: %d", *n_outs);
    cpy += sizeof(u32);

    for (i = 0; i < *n_outs; i++) {
      enum ARG_TYPE *at = cpy;
      INFO("arg type[%d]: %d", i, *at);
      cpy += sizeof(enum ARG_TYPE);
    }
  }
  printf("\n");
}

static void *pilot_ipc_create_arg_buffer_(struct map_T *map);

static void *pilot_ipc_create_arg_buffer_(struct map_T *map) {

  u32 n_entries;
  u32 size = pilot_ipc_dicc_get_size(map, &n_entries);

  if (!size) {
    WARN("No args in dicc.");
    return NULL;
  }

  c8 *res = malloc(size);

  if (!res) {
    ERROR("Could not allocate memory. Errno: %d", errno);
    return NULL;
  }

  void *res_p = res;
  // total entries
  res_p = mempcpy(res_p, &n_entries, sizeof(u32));

  struct map_it_T it;
  gen_map_it_set_begin(map, &it);
  while (it.current) {
    struct entry_c_T *entry = it.current->value;
    // ipc name

    res_p = stpcpy(res_p, (c8 *)it.current->key);
    res_p += sizeof(c8);

    // ipc params in args types
    INFO("N IN ARGS BEFORE COPY: %d", entry->e.in_args.n_args);
    res_p = mempcpy(res_p, &entry->e.in_args.n_args, sizeof(u32));
    res_p = mempcpy(res_p, entry->e.in_args.args,
                    sizeof(enum ARG_TYPE) * entry->e.in_args.n_args);

    INFO("N OUT ARGS BEFORE COPY: %d", entry->e.out_args.n_args);
    // ipc params out args types
    res_p = mempcpy(res_p, &entry->e.out_args.n_args, sizeof(u32));
    res_p = mempcpy(res_p, entry->e.out_args.args,
                    sizeof(enum ARG_TYPE) * entry->e.out_args.n_args);

    gen_map_it_get_next(map, &it);
  };

  temp_print_buffer(res, size);

  return res;
};

static void *pilot_ipc_send_dicc_(struct map_T *map);

static void *pilot_ipc_send_dicc_(struct map_T *map) {
  // size to allocate. it only takes the lenght of the args
  void *binary_stream = pilot_ipc_create_arg_buffer_(map);
  return binary_stream;

  // handler make call to render process :)
};

extern void *pilot_ipc_get_args_bs(struct map_T *map) {
  return pilot_ipc_send_dicc_(map);
};

static struct entry_T *pilot_ipc_stream_to_entry_(void **stream);

static struct entry_T *pilot_ipc_stream_to_entry_(void **stream) {

  void *s_cpy = *stream;

  u32 in_args_n;
  memcpy(&in_args_n, s_cpy, sizeof(u32));
  s_cpy += sizeof(u32);

  INFO("args %d", in_args_n);

  enum ARG_TYPE *in_args = malloc(sizeof(enum ARG_TYPE) * in_args_n);
  if (!in_args)
    goto in_err;
  memcpy(in_args, s_cpy, sizeof(enum ARG_TYPE) * in_args_n);
  s_cpy += sizeof(enum ARG_TYPE) * in_args_n;

  u32 out_args_n;
  memcpy(&out_args_n, s_cpy, sizeof(u32));
  s_cpy += sizeof(u32);

  INFO("args out %d", out_args_n);

  enum ARG_TYPE *out_args = malloc(sizeof(enum ARG_TYPE) * out_args_n);
  if (!out_args)
    goto out_err;
  memcpy(out_args, s_cpy, sizeof(enum ARG_TYPE) * out_args_n);
  s_cpy += sizeof(enum ARG_TYPE) * out_args_n;

  struct entry_T *entry = malloc(sizeof(struct entry_T));
  if (!entry)
    goto entry_err;

  entry->in_args.n_args = in_args_n;
  entry->in_args.args = in_args;

  entry->out_args.n_args = out_args_n;
  entry->out_args.args = out_args;

  *stream = s_cpy;
  return entry;

entry_err:
  free(out_args);
out_err:
  free(in_args);
in_err:
  ERROR("Failed to allocate memory (errno: %d)", errno);
  return NULL;
}

extern void pilot_ipc_stream_insert(struct map_T *map, void *stream) {

  void *s_cpy = stream;
  u32 n_entries;

  mempcpy(&n_entries, s_cpy, sizeof(u32));
  s_cpy += sizeof(u32);

  u32 i;
  INFO("n_entries: %d", n_entries);

  for (i = 0; i < n_entries; i++) {

    c8 *e_name = malloc(sizeof(c8) * (strlen((c8 *)s_cpy) + 1));
    if (!e_name) {
      ERROR("malloc failed");
      return;
    }

    INFO("entry name: %s", s_cpy);

    strcpy(e_name, (c8 *)s_cpy);
    s_cpy += strlen(e_name) + sizeof(c8);

    INFO("entry name: %s", e_name);
    struct entry_T *e = pilot_ipc_stream_to_entry_(&s_cpy);
    gen_map_insert(map, e_name, e);
  }
};
