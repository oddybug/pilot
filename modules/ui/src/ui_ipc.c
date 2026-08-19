#include "ui_ipc.h"

#include <assert.h>
#include <errno.h>
#include <memory.h>
#include <printf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data/hashmap.h"
#include "log.h"

s32 ui_ipc_entry_add(const c8 *name,
                     void (*callback)(void *data, struct response_T *response),
                     struct args_T *in_args, struct args_T *out_args) {
  assert(in_args && out_args && callback && name);

  struct map_T *map = ui_ipc_get_browser_map();
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

extern void ui_ipc_free() {
  struct map_T *map = ui_ipc_get_browser_map();
  if (!map)
    return;
  gen_map_free(map);
};

/**
 * @brief Get the size in bytes of an arg
 *
 * @param type
 * @return
 */
static size_t ui_ipc_arg_size_(enum ARG_TYPE type);

static size_t ui_ipc_arg_size_(enum ARG_TYPE type) {
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

size_t ui_ipc_argsv_get(struct args_T *args) {
  u32 res = 0;
  u32 i;
  for (i = 0; i < args->n_args; i++) {
    res += ui_ipc_arg_size_(args->args[i]);
  }
  return res;
};

extern u32 ui_ipc_stream_get_size(u32 *n_entries) {
  struct map_T *map = ui_ipc_get_browser_map();

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

extern void *ui_ipc_stream_get() {
  struct map_T *map = ui_ipc_get_browser_map();
  u32 n_entries;
  u32 size = ui_ipc_stream_get_size(&n_entries);
  if (!size)
    goto err;

  c8 *res = malloc(size);
  if (!res)
    goto err;

  void *res_p = res;
  res_p = mempcpy(res_p, &n_entries, sizeof(u32));

  struct map_it_T it;
  gen_map_it_set_begin(map, &it);
  while (it.current) {
    struct entry_c_T *entry = it.current->value;
    // ipc name
    res_p = stpcpy(res_p, (c8 *)it.current->key);
    res_p += sizeof(c8);
    // ipc params in args types
    res_p = mempcpy(res_p, &entry->e.in_args.n_args, sizeof(u32));
    res_p = mempcpy(res_p, entry->e.in_args.args,
                    sizeof(enum ARG_TYPE) * entry->e.in_args.n_args);
    // ipc params out args types
    res_p = mempcpy(res_p, &entry->e.out_args.n_args, sizeof(u32));
    res_p = mempcpy(res_p, entry->e.out_args.args,
                    sizeof(enum ARG_TYPE) * entry->e.out_args.n_args);

    gen_map_it_get_next(map, &it);
  };

  return res;

err:
  ERROR("Could not create the bitstream");
  return NULL;
};

/**
 * @brief Gets the next entry of the stream. It moves the stream pointer to the
 * next entry before return.
 *
 * @param stream reference to the void pointer
 */
static struct entry_T *ui_ipc_stream_next_entry_(void **stream);

static struct entry_T *ui_ipc_stream_next_entry_(void **stream) {
  void *s_cpy = *stream;

  u32 in_args_n;
  memcpy(&in_args_n, s_cpy, sizeof(u32));
  s_cpy += sizeof(u32);

  enum ARG_TYPE *in_args = malloc(sizeof(enum ARG_TYPE) * in_args_n);
  if (!in_args)
    goto in_err;
  memcpy(in_args, s_cpy, sizeof(enum ARG_TYPE) * in_args_n);
  s_cpy += sizeof(enum ARG_TYPE) * in_args_n;

  u32 out_args_n;
  memcpy(&out_args_n, s_cpy, sizeof(u32));
  s_cpy += sizeof(u32);

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

void ui_ipc_stream_insert(struct map_T *map, void *stream) {
  void *s_cpy = stream;
  u32 n_entries;

  mempcpy(&n_entries, s_cpy, sizeof(u32));
  s_cpy += sizeof(u32);

  u32 i;
  for (i = 0; i < n_entries; i++) {

    c8 *e_name = malloc(sizeof(c8) * (strlen((c8 *)s_cpy) + 1));
    if (!e_name) {
      ERROR("malloc failed");
      return;
    }

    strcpy(e_name, (c8 *)s_cpy);
    s_cpy += strlen(e_name) + sizeof(c8);

    struct entry_T *e = ui_ipc_stream_next_entry_(&s_cpy);
    gen_map_insert(map, e_name, e);
  }
};

void ui_ipc_stream_write_arg(void **stream, void *value, enum ARG_TYPE type) {
  void *s = *stream;
  size_t arg_size = ui_ipc_arg_size_(type);
  memcpy(s, value, arg_size);
  s += sizeof(arg_size);
  *stream = s;
};

extern void ui_ipc_stream_write_string(void **stream, const c8 *string) {
  void *s = *stream;
  size_t str_s = strlen(string);
  strcpy(s, string);
  s += str_s + sizeof(c8);
  *stream = s;
};

void ui_ipc_stream_read_arg(void **stream, void *value, enum ARG_TYPE type) {
  void *s = *stream;
  size_t arg_size = ui_ipc_arg_size_(type);
  memcpy(value, stream, arg_size);
  s += sizeof(arg_size);
  *stream = s;
};

