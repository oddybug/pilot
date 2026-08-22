#include "ui_msg_browser.h"
#include "data/hashmap.h"
#include "ui_ipc.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

/*
static size_t ui_msg_pull_bm_size(u32 *n_entries);

static size_t ui_msg_pull_bm_size(u32 *n_entries) {
  struct map_T *map = ui_msg_browser_pull_m();

  u32 res = 0;
  res += sizeof(u32); // TOTAL NUMBER OF ENTRIES

  struct map_it_T it;
  gen_map_it_set_begin(map, &it);
  *n_entries = 0;
  while (it.current) {
    struct pull_msg_e *entry = it.current->value;
    res += sizeof(enum ARG_TYPE) * entry->out.n_args;
    res += sizeof(enum ARG_TYPE) * entry->in.n_args;
    res += sizeof(u32) * 2; // n_args * 2 (in plus out)
    res += (strlen((c8 *)it.current->key) + 1) *
           sizeof(c8); // plus the size of the name +
                       // null terminator
    *n_entries += 1;
    gen_map_it_get_next(map, &it);
  };

  return res;
};
*/

msg_map_T ui_msg_pull_bm_e(struct map_it_T *it) {

  if (!(it->current && it->current))
    return NULL;

  struct pull_msg_e *entry = it->current->value;
  msg_map_T msg = ui_msg_map_create(it->current->key, &entry->in, &entry->out);
  int i = 0;

  //ui_msg_map_push_u32(msg, &entry->in.n_args);
  for (i = 0; i < entry->in.n_args; i++) {
    s32 at = entry->in.args[i];
    ui_msg_map_push_s32(msg, &at);
  }

  ui_msg_map_push_u32(msg, &entry->out.n_args);
  for (i = 0; i < entry->out.n_args; i++) {
    s32 at = entry->out.args[i];
    ui_msg_map_push_s32(msg, &at);
  }

  struct map_T *map = ui_msg_browser_pull_m();
  gen_map_it_get_next(map, it);

  return msg;
};

// TODO: rethink implementation to maybe avoid void*. This is low reusable and
// could potientaly have some implementation reusing other functions.

// For example passing small messagess and read one by one
/*
extern msg_T ui_msg_pull_bm(size_t *size) {
  assert(size);

  struct map_T *map = ui_msg_browser_pull_m();
  u32 n_entries;

  *size = ui_ipc_stream_get_size(&n_entries); // may can delete fn

  if (!size)
    goto err;

  msg_T msg;

  c8 *res = malloc(size);
  if (!res)
    goto err;

  void *res_p = res;
  res_p = mempcpy(res_p, &n_entries, sizeof(u32));

  struct map_it_T it;
  gen_map_it_set_begin(map, &it);
  while (it.current) {
    struct push_msg_e *entry = it.current->value;
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
*/
