#include "ui_ipc.h"

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define DICC_SIZE 1024

#include "data/hashmap.h"
#include "data/hashmap_helpers.h"

#include "log.h"

// Remind you this is on browser process only
struct entry_T {
  void (*callback)(void *data);
  struct args_T in_args;
  struct args_T out_args;
};

static struct map_T *e_map_;

static void pilot_entry_free_entry_(struct entry_T *e);

static void pilot_entry_free_entry_(struct entry_T *e) {
  free(e->in_args.args);
  free(e->out_args.args);
};

static void pilot_entry_free_item_fn_(struct item_T *item);

static void pilot_entry_free_item_fn_(struct item_T *item) {
  struct entry_T *value = item->value;
  pilot_entry_free_entry_(value);
};

void pilot_ipc_create_dicc() {
  e_map_ = gen_map_create(DICC_SIZE, gen_map_hash_fn_c8p, gen_map_cmp_key_c8p,
                          pilot_entry_free_item_fn_);
};

extern s32 pilot_ipc_entry_create(c8 *name, void (*callback)(void *data),
                                  struct args_T *in_args,
                                  struct args_T *out_args) {
  assert(in_args && out_args && callback && name);

  if (!gen_map_find(e_map_, name)) {
    WARN("entry already exists");
    return 2;
  }

  struct entry_T *e = malloc(sizeof(struct entry_T));
  if (!e)
    goto err_e;

  enum ARG_TYPE *in_cpy = malloc(sizeof(enum ARG_TYPE) * in_args->n_args);
  if (!in_cpy)
    goto err_in;

  memcpy(in_cpy, in_args->args, sizeof(*in_cpy) * in_args->n_args);

  enum ARG_TYPE *out_cpy = malloc(sizeof(enum ARG_TYPE) * out_args->n_args);
  if (!out_cpy)
    goto err_out;

  memcpy(out_cpy, out_args->args, sizeof(*out_cpy) * out_args->n_args);

  e->in_args = (struct args_T){.args = in_cpy, .n_args = in_args->n_args};
  e->out_args = (struct args_T){.args = out_cpy, .n_args = out_args->n_args};
  e->callback = callback;

  gen_map_insert(e_map_, name, e);
  return 0;

err_out:
  free(in_cpy);
err_in:
  free(e);
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

// wallahi this needs to be renamed and explained well because even I 5 min
// after doing it know nothing
static u32 pilot_ipc_dicc_get_size_(struct map_T *map) {

  u32 res = 0;

  struct map_it_T it;
  gen_map_it_set_begin(map, &it);
  while (it.current) {
    struct entry_T *entry = it.current->value;
    res += pilot_ipc_args_get_size_(&entry->in_args);
    res += pilot_ipc_args_get_size_(&entry->out_args);
    res += sizeof(u32) * 2;                   // n_args * 2 (in plus out)
    res += strlen((c8 *)it.current->key) + 1; // plus the size of the name +
                                              // null terminator
    gen_map_it_get_next(map, &it);
  };

  return res;
};

static void *pilot_ipc_create_arg_buffer_(struct map_T *map);

static void *pilot_ipc_create_arg_buffer_(struct map_T *map) {

  u32 size = pilot_ipc_dicc_get_size_(map);

  if (!size) {
    WARN("No args in dicc.");
    return NULL;
  }

  void *res = malloc(size);

  if (!res) {
    ERROR("Could not allocate memory. Errno: %d", errno);
    return NULL;
  }

  void *res_p = res;
  struct map_it_T it;
  gen_map_it_set_begin(map, &it);

  while (it.current) {
    struct entry_T *entry = it.current->value;
    // ipc name
    res_p = stpcpy(res_p, (c8 *)it.current->key);
    res_p += sizeof(c8);

    // ipc params in args types
    res_p = mempcpy(res_p, &entry->in_args.n_args, sizeof(u32));
    res_p = mempcpy(res_p, entry->in_args.args,
                    sizeof(enum ARG_TYPE) * entry->in_args.n_args);

    // ipc params out args types
    res_p = mempcpy(res_p, &entry->out_args.n_args, sizeof(u32));
    res_p = mempcpy(res_p, entry->out_args.args,
                    sizeof(enum ARG_TYPE) * entry->out_args.n_args);

    gen_map_it_get_next(map, &it);
  };

  return res;
};

void pilot_ipc_send_dicc_(struct map_T *map);

void pilot_ipc_send_dicc_(struct map_T *map) {
  // size to allocate. it only takes the lenght of the args
  void *binary_stream = pilot_ipc_create_arg_buffer_(map);

  // handler make call to render process :)
};
