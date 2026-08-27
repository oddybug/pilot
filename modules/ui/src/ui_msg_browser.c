
#include "ui_msg_browser.h"
#include "data/hashmap.h"
#include "log.h"
#include "ui_msg_common.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

s32 ui_msg_pull_new_entry(const c8 *name,
                          void (*callback)(msg_T msg, msg_T response),
                          struct args *in, struct args *out) {
  assert(in && out && callback && name);

  struct map_T *map = ui_msg_browser_pull_m();
  if (gen_map_find(map, name)) {
    WARN("entry already exists");
    return 2;
  }

  struct pull_msg_bme *e_c = malloc(sizeof(struct pull_msg_bme));
  if (!e_c)
    goto err_e;

  enum ARG_TYPE *in_cpy = malloc(sizeof(enum ARG_TYPE) * in->n_args);
  if (!in_cpy)
    goto err_in;
  memcpy(in_cpy, in->args, sizeof(*in_cpy) * in->n_args);

  enum ARG_TYPE *out_cpy = malloc(sizeof(enum ARG_TYPE) * out->n_args);
  if (!out_cpy)
    goto err_out;
  memcpy(out_cpy, out->args, sizeof(*out_cpy) * out->n_args);

  e_c->callback = callback;
  e_c->in = (struct args){.args = in_cpy, .n_args = in->n_args};
  e_c->out = (struct args){.args = out_cpy, .n_args = out->n_args};

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

msg_T ui_msg_pull_create(const c8 *name) {
  struct map_T *map = ui_msg_browser_pull_m();
  struct pull_msg_bme *pme = gen_map_find(map, name);
  msg_T msg = ui_msg_create_(name, &pme->out);
  return msg;
};

msg_map_T ui_msg_pull_bm_e(struct map_it_T *it) {

  if (!(it->current && it->current))
    return NULL;

  struct pull_msg_bme *entry = it->current->value;
  msg_map_T msg = ui_msg_map_create(it->current->key, &entry->in, &entry->out);
  int i = 0;

  // ui_msg_map_push_u32(msg, &entry->in.n_args);
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
