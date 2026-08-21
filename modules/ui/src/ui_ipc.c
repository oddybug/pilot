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

struct msg {
  struct args *args;
  void *msg;
  void *it;
  u32 i;
};

struct msg_map {
  void *msg;
  void *it;
  size_t size;
};

const c8 *ui_args_e2s_(enum ARG_TYPE type);

const c8 *ui_args_e2s_(enum ARG_TYPE type) {
  switch (type) {
  case S32:
    return "S32";
    break;
  case U32:
    return "U32";
    break;
  case ARG_TYPE:
    return "ARG_TYPE";
    break;
  default:
    return "[WRONG_TYPE_PROVIDED]";
  }
};

/**
 * @brief Get the size in bytes of an arg
 *
 * @param type
 * @return
 */
static size_t ui_args_arg_size_(enum ARG_TYPE type);

static size_t ui_args_arg_size_(enum ARG_TYPE type) {
  u32 res;
  switch (type) {
  case U32:
    res = sizeof(u32);
    break;
  case S32:
    res = sizeof(s32);
    break;
  case ARG_TYPE:
    res = sizeof(enum ARG_TYPE);
    break;
  }
  return res;
};

size_t ui_args_argsv_get(struct args *args) {
  u32 res = 0;
  u32 i;
  for (i = 0; i < args->n_args; i++) {
    res += ui_args_arg_size_(args->args[i]);
  }
  return res;
};

void ui_msg_map_push_s32(msg_map_T msg, s32 *val) {
  memcpy(msg->it, val, sizeof(s32));
  msg->it += sizeof(s32);
};

void ui_msg_map_push_u32(msg_map_T msg, u32 *val) {
  memcpy(msg->it, val, sizeof(u32));
  msg->it += sizeof(u32);
};

void ui_msg_map_read_s32(msg_map_T msg, s32 *val) {
  memcpy(val, msg->it, sizeof(s32));
  msg->it += sizeof(s32);
};

extern void ui_msg_map_read_u32(msg_map_T msg, u32 *val) {
  memcpy(val, msg->it, sizeof(u32));
  msg->it += sizeof(u32);
};

msg_map_T ui_msg_map_create(const c8 *name, struct args *in, struct args *out) {
  assert(in || out || name);

  msg_map_T msg = malloc(sizeof(struct msg_map));
  if (!msg)
    goto err;

  // name + null terminator + 2 * nargs + argv in size + argv out size
  size_t msg_size = strlen(name) + sizeof(c8) + sizeof(s32) * 2 +
                    ui_args_argsv_get(in) + ui_args_argsv_get(out);
  msg->size = msg_size;

  msg->msg = malloc(msg_size);
  if (!msg->msg)
    goto err_name;

  strcpy(msg->msg, name);
  msg->it += strlen(name) + sizeof(c8);

  return msg;
err_name:
  free(msg);
err:
  return NULL;
};

void ui_msg_map_free(msg_map_T msg) {
  assert(msg || msg->msg);
  free(msg->msg);
  free(msg);
};

void *ui_msg_map_bs(msg_map_T msg) {
  assert(msg);
  return msg->msg;
};

msg_map_T ui_msg_map_bs2m(void *stream, size_t size) {
  msg_map_T msg = malloc(sizeof(struct msg_map));
  if (!msg) {
    goto err;
  }
  msg->msg = malloc(size);
  if (!msg->msg) {
    goto err_name;
  }
  mempcpy(msg->msg, stream, size);
  msg->size = size;
  msg->it = msg->msg + strlen(stream) + sizeof(c8);
  return msg;

err_name:
  free(msg);
err:
  return NULL;
};

size_t ui_msg_map_size(msg_map_T msg) {
  assert(msg || msg->msg);
  return msg->size;
};

s32 ui_msg_pull_new_entry(const c8 *name,
                          void (*callback)(msg_T msg, msg_T response),
                          struct args *in, struct args *out) {
  assert(in && out && callback && name);

  struct map_T *map = ui_msg_browser_pull_m();
  if (gen_map_find(map, name)) {
    WARN("entry already exists");
    return 2;
  }

  struct pull_msg_e *e_c = malloc(sizeof(struct pull_msg_e));
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

extern msg_T ui_msg_create_(const c8 *name, struct args *args);

msg_T ui_msg_create_(const c8 *name, struct args *args) {
  msg_T msg = malloc(sizeof(struct msg));
  if (!msg)
    goto err;

  size_t msg_size = ui_args_argsv_get(args) + sizeof(name) + sizeof(c8);
  msg->msg = malloc(msg_size);
  if (!msg->msg)
    goto err_name;
  strcpy(msg->msg, name);

  msg->it = msg + strlen(name) + 2 * sizeof(c8);
  msg->i = 0;

  return msg;
err_name:
  free(msg);
err:
  return NULL;
};

extern msg_T ui_msg_get_fs(void *stream, struct args *args) {
  msg_T msg;
  msg->args = args;
  size_t msg_size = ui_args_argsv_get(args) + strlen(stream) + sizeof(c8);
  msg->msg = malloc(msg_size);
  if (!msg->msg)
    return NULL;
  memcpy(msg->msg, stream, msg_size);
  msg->it = msg + strlen(stream) + 2 * sizeof(c8);
  msg->i = 0;
  return msg;
};

extern size_t ui_msg_size(msg_T msg) {
  size_t msg_size =
      ui_args_argsv_get(msg->args) + strlen(msg->msg) + sizeof(c8);
  return msg_size;
};

extern void *ui_msg_bitstream(msg_T msg) { return msg->msg; };

s32 ui_msg_arg_read_s32(msg_T msg, s32 *val) {
  if (msg->i > msg->args->n_args) {
    WARN("No more arguments to read in %s", msg->msg);
    return 1;
  }
  enum ARG_TYPE t = msg->args->args[msg->i];
  if (t != S32) {
    WARN("Tried to read S32 when next argument is %s from %s", ui_args_e2s_(t),
         msg->msg);
    return 1;
  }

  memcpy(val, msg->it, sizeof(s32));
  msg->it += sizeof(s32);
  msg->i++;
  return 0;
};

s32 ui_msg_arg_read_u32(msg_T msg, u32 *val) {
  if (msg->i > msg->args->n_args) {
    WARN("No more arguments to read in %s", msg->msg);
    return 1;
  }
  enum ARG_TYPE t = msg->args->args[msg->i];
  if (t != U32) {
    WARN("Tried to read U32 when next argument is %s from %s", ui_args_e2s_(t),
         msg->msg);
    return 1;
  }

  memcpy(val, msg->it, sizeof(u32));
  msg->it += sizeof(u32);
  msg->i++;
  return 0;
};

extern void *ui_msg_arg_read(msg_T msg) {
  switch (msg->args->args[msg->i]) {
  S32: {
    s32 *val;
    ui_msg_arg_read_s32(msg, val);
    return (void *)val;
    break;
  }
  U32: {
    u32 *val;
    ui_msg_arg_read_u32(msg, val);
    return (void *)val;
    break;
  }
  ARG_TYPE:
    return NULL;
    break;
  default:

    return NULL;
    break;
  }
};

extern msg_T ui_msg_pull_create(const c8 *name) {
  struct map_T *map = ui_msg_browser_pull_m();
  struct pull_msg_e *pme = gen_map_find(map, name);
  msg_T msg = ui_msg_create_(name, &pme->out);
  return msg;
};

extern void ui_msg_free(msg_T msg) {
  assert(msg || msg->msg);
  free(msg->msg);
  free(msg);
};

s32 ui_msg_push_s32(msg_T msg, s32 val) {
  if (msg->i > msg->args->n_args) {
    WARN("No more arguments to push in %s", msg->msg);
    return 1;
  }
  enum ARG_TYPE t = msg->args->args[msg->i];
  if (t != S32) {
    WARN("Tried to push S32 when next argument is %s from %s", ui_args_e2s_(t),
         msg->msg);
    return 1;
  }
  memcpy(msg->it, &val, sizeof(s32));

  msg->it += sizeof(s32);
  msg->i++;
  return 0;
};

extern s32 ui_msg_push_u32(msg_T msg, u32 val) {
  if (msg->i > msg->args->n_args) {
    WARN("No more arguments to push in %s", msg->msg);
    return 1;
  }
  enum ARG_TYPE t = msg->args->args[msg->i];
  if (t != U32) {
    WARN("Tried to push U32 when next argument is %s from %s", ui_args_e2s_(t),
         msg->msg);
    return 1;
  }
  memcpy(msg->it, &val, sizeof(u32));

  msg->it += sizeof(u32);
  msg->i++;
  return 0;
};

extern void *ui_msg_arg_read(msg_T msg);

extern void ui_ipc_free() {
  struct map_T *map = ui_msg_browser_pull_m();
  if (!map)
    return;
  gen_map_free(map);
};

/*
extern u32 ui_ipc_stream_get_size(u32 *n_entries) {
  struct map_T *map = ui_msg_browser_pull_m();

  u32 res = 0;
  res += sizeof(u32); // TOTAL NUMBER OF ENTRIES

  struct map_it_T it;
  gen_map_it_set_begin(map, &it);
  *n_entries = 0;
  while (it.current) {
    struct push_msg_e *entry = it.current->value;
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
*/

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
/*
extern void *ui_ipc_stream_get() {
  struct map_T *map = ui_msg_browser_pull_m();
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

/**
 * @brief Gets the next entry of the stream. It moves the stream pointer to the
 * next entry before return.
 *
 * @param stream reference to the void pointer
 */
// static struct push_msg_e_render *ui_ipc_stream_next_entry_(void **stream);
/*
static struct push_msg_e_render *ui_ipc_stream_next_entry_(void **stream) {
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

  struct push_msg_e_render *entry = malloc(sizeof(struct push_msg_e_render));
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

  memcpy(&n_entries, s_cpy, sizeof(u32));
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

    struct push_msg_e_render *e = ui_ipc_stream_next_entry_(&s_cpy);
    gen_map_insert(map, e_name, e);
  }
};

void ui_ipc_stream_write_arg(void **stream, void *value, enum ARG_TYPE type) {
  void *s = *stream;
  size_t arg_size = ui_args_arg_size_(type);
  memcpy(s, value, arg_size);
  s += sizeof(arg_size);
  *stream = s;
};

void ui_ipc_stream_write_string(void **stream, const c8 *string) {
  void *s = *stream;
  size_t str_s = strlen(string);
  strcpy(s, string);
  s += str_s + sizeof(c8);
  *stream = s;
};

void ui_ipc_stream_read_arg(void **stream, void *value, enum ARG_TYPE type) {
  void *s = *stream;
  size_t arg_size = ui_args_arg_size_(type);
  memcpy(value, s, arg_size);
  s += sizeof(arg_size);
  *stream = s;
};

*/
