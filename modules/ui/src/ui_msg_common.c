#include "ui_msg_common.h"

#include <assert.h>
#include <errno.h>
#include <memory.h>
#include <printf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data/hashmap.h"
#include "log.h"
#include "ui.h"
#include "ui_msg_browser.h"

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
  INFO("h");
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
  msg->it = msg->msg + strlen(name) + sizeof(c8);

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

msg_T ui_msg_create_(const c8 *name, struct args *args) {
  assert(args && name);
  msg_T msg = malloc(sizeof(struct msg));
  if (!msg)
    goto err;

  size_t msg_size = ui_args_argsv_get(args) + sizeof(name) + sizeof(c8);
  msg->msg = malloc(msg_size);
  if (!msg->msg)
    goto err_name;
  strcpy(msg->msg, name);

  msg->it = msg->msg + strlen(name) + 2 * sizeof(c8);
  msg->i = 0;
  msg->args = args;

  return msg;
err_name:
  free(msg);
err:
  return NULL;
};

extern msg_T ui_msg_get_fs(void *stream, struct args *args) {
  msg_T msg = malloc(sizeof(struct msg));
  if (!msg)
    return NULL;
  msg->args = args;
  size_t msg_size = ui_args_argsv_get(args) + strlen(stream) + sizeof(c8);
  msg->msg = malloc(msg_size);
  if (!msg->msg)
    return NULL;
  memcpy(msg->msg, stream, msg_size);
  msg->it = msg->msg + strlen(stream) + 2 * sizeof(c8);
  msg->i = 0;
  return msg;
};

extern msg_T ui_msg_get_fs_raw(void *stream, size_t size) {
  msg_T msg = malloc(sizeof(struct msg));
  if (!msg)
    return NULL;
  msg->args = NULL;
  msg->msg = malloc(size);
  if (!msg->msg)
    return NULL;
  memcpy(msg->msg, stream, size);
  msg->it = msg->msg + strlen(stream) + 2 * sizeof(c8);
  msg->i = 0;
  return msg;
};

extern size_t ui_msg_size(msg_T msg) {
  size_t msg_size =
      ui_args_argsv_get(msg->args) + strlen(msg->msg) + sizeof(c8);
  return msg_size;
};

extern void *ui_msg_bitstream(msg_T msg) { return msg->msg; };

extern msg_T ui_msg_push_create(const c8 *name) {
  map_T map = ui_msg_browser_push_m();
  struct push_msg_bme *pmbme = (struct push_msg_bme *)gen_map_find(map, name);
  if (!pmbme) {
    WARN("'%s' is not registered.", name);
    return NULL;
  }

  msg_T msg = ui_msg_create_(name, &pmbme->out);
  return msg;
};

s32 ui_msg_arg_read_s32(msg_T msg, s32 *val) {
  assert(msg);
  assert(msg->msg);
  assert(msg->args);
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
  INFO("number: %d value: %d", *(s32 *)msg->it, *(s32 *)val);
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

void ui_msg_arg_read(msg_T msg, void *val) {
  switch (msg->args->args[msg->i]) {
  S32:
    {
      ui_msg_arg_read_s32(msg, val);
      break;
    }
  U32:
    {
      ui_msg_arg_read_u32(msg, val);
      break;
    }
  ARG_TYPE:
    break;
  default:

    break;
  }
};

extern void ui_msg_free(msg_T msg) {
  assert(msg || msg->msg);
  free(msg->msg);
  free(msg);
};

extern void ui_msg_cpy_name(msg_T msg, c8 *name) { strcpy(name, msg->msg); };

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

void ui_msg_push_s32_r(msg_T msg, s32 val) {
  memcpy(msg->it, &val, sizeof(s32));
  msg->it += sizeof(s32);
};

void ui_msg_push_u32_r(msg_T msg, u32 val) {
  memcpy(msg->it, &val, sizeof(u32));
  msg->it += sizeof(u32);
};

extern void ui_msg_arg_push_string_r(msg_T msg, const c8 *string) {

  strcpy(msg->it, string);
  msg->it += sizeof(c8) * (2 + strlen(string));
};

void ui_msg_read_s32_r(msg_T msg, s32 *val) {
  memcpy(val, msg->it, sizeof(s32));
  msg->it += sizeof(s32);
};

void ui_msg_read_u32_r(msg_T msg, u32 *val) {
  memcpy(val, msg->it, sizeof(u32));
  msg->it += sizeof(u32);
};

void ui_msg_read_string_r(msg_T msg, c8 *string) {
  strcpy(string, msg->it);
  msg->it += sizeof(c8) * (2 + strlen(string));
};

size_t ui_msg_string_size_r(msg_T msg) { return strlen(msg->it); };
