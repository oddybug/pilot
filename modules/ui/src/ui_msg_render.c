#include "ui_msg_render.h"
#include "data/hashmap.h"
#include "log.h"
#include <stdlib.h>
#include <string.h>

void ui_msg_pullme_free(struct pull_msg_e_render *e) {
  free(e->in.args);
  free(e->out.args);
};

// void ui_msg_pushme_free(struct pull_msg_e_render *e) { free(e->out.args); };

static struct pull_msg_e_render *ui_msg_mce_render(msg_map_T msg) {
  struct pull_msg_e_render *entry = malloc(sizeof(struct pull_msg_e_render));
  if (!entry) {
    goto err;
  }

  int i;

  u32 n_in;
  ui_msg_map_read_u32(msg, &n_in);

  enum ARG_TYPE *in;
  if (n_in)
    in = malloc(n_in * sizeof(enum ARG_TYPE));
  else
    in = NULL;

  if (!in && !n_in)
    goto err_in;
  for (i = 0; i < n_in; i++) {
    enum ARG_TYPE type;
    ui_msg_map_read_s32(msg, (s32 *)&type);
    in[i] = type;
  }
  entry->in = (struct args){.args = in, .n_args = n_in};

  u32 n_out;
  ui_msg_map_read_u32(msg, &n_out);

  enum ARG_TYPE *out;
  if (n_out)
    out = malloc(n_out * sizeof(enum ARG_TYPE));
  else
    out = NULL;

  if (!out && !n_out)
    goto err_out;
  for (i = 0; i < n_out; i++) {
    enum ARG_TYPE type;
    ui_msg_map_read_s32(msg, (s32 *)&type);
    out[i] = type;
  }
  entry->out = (struct args){.args = out, .n_args = n_out};

  return entry;
err_out:
  free(in);
err_in:
  free(entry);
err:
  return NULL;
};

s32 ui_msg_pull_rm_add(void *stream, size_t size) {
  map_T map = ui_msg_render_pull_m();
  if (!map)
    goto err;

  msg_map_T msg = ui_msg_map_bs2m(stream, size);
  if (!msg)
    goto err;

  struct pull_msg_e_render *entry = ui_msg_mce_render(msg);
  if (!entry)
    goto err;

  void *bs = ui_msg_map_bs(msg);
  if (!bs)
    goto err;

  c8 *key = malloc(strlen(bs) + sizeof(c8));
  if (!key)
    goto err;
  strcpy(key, bs);

  s32 err = gen_map_insert(map, key, entry);
  ui_msg_map_free(msg);
  return 0;
err:
  WARN("failed to add entry to pull msg in renderer");
  return 1;
};

extern msg_T ui_msg_create_(const c8 *name, struct args *args);

msg_T ui_msg_pull_render_create(c8 *name) {
  map_T map = ui_msg_render_pull_m();
  struct pull_msg_e_render *pme = gen_map_find(map, name);
  msg_T msg = ui_msg_create_(name, &pme->in);
  return msg;
};

msg_T ui_msg_push_request(const c8 *name, struct args *args) {
  msg_T msg = ui_msg_create_(name, args);
  return msg;
};
