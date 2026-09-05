#include "ui_msg_browser.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "data/list.h"
#include "ui.h"
#include "ui_msg_common.h"

#include "data/hashmap.h"
#include "log.h"

void ui_msg_pullem_free_clbk_(struct item_T *item) {
  struct pull_msg_bme *value = item->value;
  ui_msg_pullem_free(value);
};

void ui_msg_pullem_free(struct pull_msg_bme *e) {
  free(e->out.args);
  free(e->in.args);
};

void ui_msg_pushem_free(struct push_msg_bme *e) {
  // TODO: URGENT FREE LIST
  // gen_list_free(e->list);
  free(e->out.args);
};

void ui_msg_pushem_free_clbk_(struct item_T *item) {

  struct push_msg_bme *value = item->value;
  ui_msg_pushem_free(value);
};

s32 ui_msg_pull_new_entry(const c8 *name,
                          void (*callback)(msg_T msg, msg_T response),
                          struct args *in, struct args *out) {
  assert(in && out && callback && name);

  map_T map = ui_msg_browser_pull_m();
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

extern s32 ui_msg_push_new_entry(const c8 *name, struct args *out) {
  assert(out && name);

  map_T map = ui_msg_browser_push_m();
  if (gen_map_find(map, name)) {
    // USER WARN
    WARN("entry already exists");
    return 2;
  }

  struct push_msg_bme *e_c = malloc(sizeof(struct push_msg_bme));
  if (!e_c)
    goto err_e;

  enum ARG_TYPE *out_cpy = malloc(sizeof(enum ARG_TYPE) * out->n_args);
  if (!out_cpy)
    goto err_out;
  memcpy(out_cpy, out->args, sizeof(*out_cpy) * out->n_args);

  e_c->out = (struct args){.args = out_cpy, .n_args = out->n_args};
  e_c->render = NULL;

  gen_map_insert(map, name, e_c);
  return 0;

err_out:
  free(e_c);
err_e:
  WARN("failed to allocate memory for new entry");
  return 1;
};

// msg_T ui_msg_pull_create(const c8 *name) {
//   map_T map = ui_msg_browser_pull_m();
//   struct pull_msg_bme *pme = gen_map_find(map, name);
//   msg_T msg = ui_msg_create_(name, &pme->out);
//   return msg;
// };

static void ui_msg_args_copy(struct args *dest, struct args *src,
                             size_t offset) {
  if (src->n_args + offset > dest->n_args) {
    WARN("tried to copy more elements than the buffer can hold");
    return;
  };
  memcpy(dest->args + offset, src->args, src->n_args * sizeof(enum ARG_TYPE));
};

msg_T ui_msg_pull_bm_e(struct map_it_T *it) {

  if (!(it->current && it->current))
    return NULL;

  struct pull_msg_bme *entry = it->current->value;
  // msg_T msg = ui_msg_map_create(it->current->key, &entry->in, &entry->out);
  s32 args_s = entry->in.n_args + entry->out.n_args + 2;
  enum ARG_TYPE args_t[args_s];
  struct args args = {.n_args = args_s, .args = args_t};

  args.args[0] = S32;
  ui_msg_args_copy(&args, &entry->in, 1);
  args.args[1 + entry->in.n_args] = S32;
  ui_msg_args_copy(&args, &entry->out, 2 + entry->in.n_args);

  msg_T msg = ui_msg_create(it->current->key, &args);

  list_T values = gen_list_new();
  gen_list_push_back(values, &entry->in.n_args);

  s32 i;
  for (i = 0; i < entry->in.n_args; i++) {
    gen_list_push_back(values, &entry->in.args[i]);
  }

  gen_list_push_back(values, &entry->out.n_args);

  for (i = 0; i < entry->out.n_args; i++) {
    gen_list_push_back(values, &entry->out.args[i]);
  }

  ui_msg_populate_r(msg, values);
  // Create msg.

  // list now

  // msg_T msg = ui_msg_create(it->current->key, &args);
  map_T map = ui_msg_browser_pull_m();
  gen_map_it_get_next(map, it);

  return msg;
};
