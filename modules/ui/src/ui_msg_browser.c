#include "ui_msg_browser.h"

#include <assert.h>
#include <stdarg.h>
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

  enum ARG_TYPE *in_cpy = NULL;
  if (in->n_args > 0) {
    if (!in->args)
      goto err_in;
    in_cpy = malloc(sizeof(enum ARG_TYPE) * in->n_args);
    if (!in_cpy)
      goto err_in;
    memcpy(in_cpy, in->args, sizeof(*in_cpy) * in->n_args);
  }

  enum ARG_TYPE *out_cpy = NULL;
  if (out->n_args > 0) {
    if (!out->args)
      goto err_out;
    out_cpy = malloc(sizeof(enum ARG_TYPE) * out->n_args);
    if (!out_cpy)
      goto err_out;
    memcpy(out_cpy, out->args, sizeof(*out_cpy) * out->n_args);
  }

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

s32 ui_msg_push_new_entry(const c8 *name, struct args *out) {
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

  enum ARG_TYPE *out_cpy = NULL;
  if (out->n_args > 0) {
    if (!out->args)
      goto err_out;
    out_cpy = malloc(sizeof(enum ARG_TYPE) * out->n_args);
    if (!out_cpy)
      goto err_out;
    memcpy(out_cpy, out->args, sizeof(enum ARG_TYPE) * out->n_args);
  }

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

#define UI_MSG_MAX_ARGS 256

s32 ui_msg_pull_new(const c8 *name,
                           void (*callback)(msg_T msg, msg_T response)) {
  assert(name && callback);

  map_T map = ui_msg_browser_pull_m();
  if (gen_map_find(map, name)) {
    WARN("entry already exists");
    return 2;
  }

  struct pull_msg_bme *e_c = malloc(sizeof(struct pull_msg_bme));
  if (!e_c) {
    WARN("failed to allocate memory for new entry");
    return 1;
  }

  e_c->callback = callback;
  e_c->in = (struct args){.args = NULL, .n_args = 0};
  e_c->out = (struct args){.args = NULL, .n_args = 0};

  gen_map_insert(map, name, e_c);
  return 0;
};

static s32 ui_msg_pull_set_h(struct args *dst, va_list list) {
  va_list count_l;
  va_copy(count_l, list);
  u32 n = 0;
  int v;
  do {
    if (n > UI_MSG_MAX_ARGS) {
      WARN("argument list is not terminated (missing ARG_END?)");
      va_end(count_l);
      return 1;
    }
    v = va_arg(count_l, int);
    if (v == (int)ARG_END)
      break;
    n++;
  } while (1);
  va_end(count_l);

  enum ARG_TYPE *cpy = NULL;
  if (n > 0) {
    cpy = malloc(sizeof(enum ARG_TYPE) * n);
    if (!cpy) {
      WARN("failed to allocate memory for entry args");
      return 1;
    }
    va_list fill_l;
    va_copy(fill_l, list);
    for (u32 i = 0; i < n; i++) {
      v = va_arg(fill_l, int);
      cpy[i] = (enum ARG_TYPE)v;
    }
    va_end(fill_l);
  }

  free(dst->args);
  dst->args = cpy;
  dst->n_args = n;
  return 0;
};

s32 ui_msg_pull_set_i_h(const c8 *name, ...) {
  assert(name);
  map_T map = ui_msg_browser_pull_m();
  struct pull_msg_bme *e = (struct pull_msg_bme *)gen_map_find(map, name);
  if (!e) {
    WARN("entry '%s' does not exist (call ui_msg_pull_new first)", name);
    return 3;
  }
  va_list list;
  va_start(list, name);
  s32 rc = ui_msg_pull_set_h(&e->in, list);
  va_end(list);
  return rc;
};

s32 ui_msg_pull_set_o_h(const c8 *name, ...) {
  assert(name);
  map_T map = ui_msg_browser_pull_m();
  struct pull_msg_bme *e = (struct pull_msg_bme *)gen_map_find(map, name);
  if (!e) {
    WARN("entry '%s' does not exist (call ui_msg_pull_new first)", name);
    return 3;
  }
  va_list list;
  va_start(list, name);
  s32 rc = ui_msg_pull_set_h(&e->out, list);
  va_end(list);
  return rc;
};

msg_T ui_msg_pull_bm_e(struct map_it_T *it) {

  if (!(it->current && it->current))
    return NULL;

  struct pull_msg_bme *entry = it->current->value;
  s32 args_s = entry->in.n_args + entry->out.n_args + 2;
  enum ARG_TYPE args_t[args_s];
  struct args args = {.n_args = args_s, .args = args_t};

  s32 i;
  args.args[0] = S32;
  for (i = 0; i < entry->in.n_args; i++)
    args.args[1 + i] = ARG_TYPE;
  args.args[1 + entry->in.n_args] = S32;
  for (i = 0; i < entry->out.n_args; i++)
    args.args[2 + entry->in.n_args + i] = ARG_TYPE;

  msg_T msg = ui_msg_create(it->current->key, &args);

  list_T values = gen_list_new();
  gen_list_push_back(values, &entry->in.n_args);

  for (i = 0; i < entry->in.n_args; i++) {
    gen_list_push_back(values, &entry->in.args[i]);
  }

  gen_list_push_back(values, &entry->out.n_args);

  for (i = 0; i < entry->out.n_args; i++) {
    gen_list_push_back(values, &entry->out.args[i]);
  }

  ui_msg_populate_r(msg, values);

  map_T map = ui_msg_browser_pull_m();
  gen_map_it_get_next(map, it);

  return msg;
};
