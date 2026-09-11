#include "ui_msg_common.h"

#include <assert.h>

#include <errno.h>
#include <memory.h>
#include <printf.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data/hashmap.h"
#include "data/list.h"
#include "log.h"
#include "ui.h"
#include "ui_msg_browser.h"

struct msg {
  c8 *name;
  enum MSG_ACCESS access;
  struct args args;
  void *msg;
  void *it;
  u32 i;
  size_t size;
};

msg_T ui_msg_create(const c8 *name, struct args *args) {
  assert(name);
  msg_T msg = malloc(sizeof(struct msg));
  if (!msg)
    goto err;

  msg->name = malloc(strlen(name) + sizeof(c8));
  if (!name)
    goto err_name;
  strcpy(msg->name, name);

  msg->args.args = malloc(sizeof(enum ARG_TYPE) * args->n_args);

  if (!msg->args.args)
    goto err_args;

  memcpy(msg->args.args, args->args, sizeof(enum ARG_TYPE) * args->n_args);
  msg->args.n_args = args->n_args;
  msg->size = 0;
  msg->msg = NULL;
  msg->it = NULL;
  msg->access = MSG_WRITE;
  msg->i = 0;

  return msg;

err_args:
  free(msg->name);
err_name:
  free(msg);
err:
  WARN("could not create message '%s'", name);
  return NULL;
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
  case STRING:
    return "STRING";
    break;
  case ARG_TYPE:
    return "ARG_TYPE";
    break;
  case ARG_END:
    return "ARG_END";
    break;
  default:
    return "[WRONG_TYPE_PROVIDED]";
  }
};

static size_t ui_args_arg_size_(enum ARG_TYPE type, void *value) {
  u32 res;
  switch (type) {
  case U32:
    res = sizeof(u32);
    break;
  case S32:
    res = sizeof(s32);
    break;
  case STRING:
    res = strlen(value) + 1;
    break;
  case ARG_TYPE:
    res = sizeof(enum ARG_TYPE);
    break;
  }
  return res;
};

static size_t ui_args_argsv_get_(struct args *args, va_list list);

static size_t ui_args_argsv_get_(struct args *args, va_list list) {

  size_t msg_size = 0;
  for (int i = 0; i < args->n_args; i++) {
    switch (args->args[i]) {
    case U32:
      (void)va_arg(list, u32);
      msg_size += ui_args_arg_size_(U32, NULL);
      break;
    case S32:
      (void)va_arg(list, s32);
      msg_size += ui_args_arg_size_(S32, NULL);
      break;
    case ARG_TYPE:
      (void)va_arg(list, s32);
      msg_size += ui_args_arg_size_(ARG_TYPE, NULL);
      break;
    case STRING:
      msg_size += ui_args_arg_size_(STRING, va_arg(list, c8 *));
      break;
    case ARG_END:
      assert(0);
      WARN("ARG_END is not a valid argument");
      msg_size = 0;
      break;
    }
  }
  return msg_size;
};

static size_t ui_args_argsv_get_r_(struct args *args, list_T list);

static size_t ui_args_argsv_get_r_(struct args *args, list_T list) {
  size_t msg_size = 0;
  if (gen_list_size(list) != args->n_args) {
    WARN("number of args dont match with list size");
    return 0;
  }

  struct node *value = gen_list_first(list);

  for (int i = 0; i < args->n_args; i++) {
    switch (args->args[i]) {
    case U32:
      msg_size += ui_args_arg_size_(U32, NULL);
      break;
    case S32:
      msg_size += ui_args_arg_size_(S32, NULL);
      break;
    case ARG_TYPE:
      msg_size += ui_args_arg_size_(ARG_TYPE, NULL);
      break;
    case STRING:
      msg_size += ui_args_arg_size_(STRING, (c8 *)value);
      break;
    }
    value = value->next;
  }
  return msg_size;
};

static void ui_msg_populate_h(msg_T msg, size_t args_s, va_list l);

static void ui_msg_populate_h(msg_T msg, size_t args_s, va_list l) {
  assert(msg && !msg->msg);

  msg->msg = malloc(args_s + strlen(msg->name) + sizeof(c8));

  if (!msg->msg)
    goto err_name;
  strcpy(msg->msg, msg->name);

  msg->it = msg->msg + strlen(msg->name) + 1 * sizeof(c8);

  s32 i;
  for (i = 0; i < msg->args.n_args; i++) {
    switch (msg->args.args[i]) {
    case U32: {
      u32 value = va_arg(l, u32);
      ui_msg_write_u32_r(msg, value);
      break;
    }
    case S32: {
      s32 value = va_arg(l, s32);
      ui_msg_write_s32_r(msg, value);
      break;
    }
    case ARG_TYPE: {
      s32 value = (s32)va_arg(l, int);
      ui_msg_write_s32_r(msg, value);
      break;
    }
    case STRING: {
      c8 *value = va_arg(l, c8 *);
      if (!value)
        goto err_fill;
      ui_msg_write_string_r(msg, value);
      break;
    }
    }
    msg->i++;
  }
  msg->size = strlen(msg->name) + 1 + args_s;
  return;
err_fill:
  free(msg->msg);
err_name:
  return;
};

static void ui_msg_populate_hr_(msg_T msg, size_t args_s, list_T l);

static void ui_msg_populate_hr_(msg_T msg, size_t args_s, list_T l) {
  assert(msg && !msg->msg && l);

  if (msg->access != MSG_WRITE) {
    ERROR("Tried to write in a read message");
    goto err_access;
  }

  if (gen_list_size(l) != msg->args.n_args) {
    ERROR("args size and list size are not equal");
    goto err_size;
  }

  msg->msg = malloc(args_s + strlen(msg->name) + sizeof(c8));
  if (!msg->msg)
    goto err_name;
  strcpy(msg->msg, msg->name);

  msg->it = msg->msg + strlen(msg->name) + 1 * sizeof(c8);

  struct node *n = gen_list_first(l);

  s32 i;
  for (i = 0; i < msg->args.n_args; i++) {
    switch (msg->args.args[i]) {
    case U32: {
      ui_msg_write_u32_r(msg, *(u32 *)n->value);
      break;
    }
    case S32: {
      ui_msg_write_s32_r(msg, *(s32 *)n->value);
      break;
    }
    case ARG_TYPE: {
      ui_msg_write_s32_r(msg, *(s32 *)n->value);
      break;
    }
    case STRING: {
      c8 *value = (c8 *)n->value;
      if (!value)
        goto err_fill;
      ui_msg_write_string_r(msg, value);
      break;
    }
    case ARG_END:
      assert(0);
      WARN("ARG_END is not a valid argument");
      break;
    }
    n = n->next;
    msg->i++;
  }
  msg->size = args_s + strlen(msg->name) + sizeof(c8);
  return;
err_fill:
  free(msg->msg);
err_name:
err_size:
err_access:
  return;
}

void ui_msg_populate_(msg_T msg, ...) {
  assert(msg);
  assert(msg->access == MSG_WRITE);
  assert(msg->args.args);
  va_list list;
  va_start(list, msg);
  size_t args_s = ui_args_argsv_get_(&msg->args, list);
  va_end(list);
  va_start(list, msg);
  ui_msg_populate_h(msg, args_s, list);
  va_end(list);
};

void ui_msg_populate_r(msg_T msg, list_T list) {
  assert(msg);
  size_t args_s = ui_args_argsv_get_r_(&msg->args, list);
  ui_msg_populate_hr_(msg, args_s, list);
};

msg_T ui_msg_get_fs(void *stream, size_t stream_s, struct args *args) {
  msg_T msg = ui_msg_get_fs_r(stream, stream_s);
  msg->args.n_args = args->n_args;
  msg->args.args = malloc(sizeof(enum ARG_TYPE) * args->n_args);
  if (!msg->args.args) {
    ERROR("malloc failed");
    free(msg);
    return NULL;
  }
  memcpy(msg->args.args, args->args, sizeof(enum ARG_TYPE) * args->n_args);
  return msg;
};

msg_T ui_msg_get_fs_r(void *stream, size_t size) {
  assert(stream);

  msg_T msg = malloc(sizeof(struct msg));
  if (!msg)
    goto err_msg;

  msg->name = malloc(strlen(stream) + sizeof(c8));
  if (!msg->name)
    goto err_name;
  strcpy(msg->name, stream);

  msg->msg = malloc(size);
  if (!msg->msg)
    goto err_msg_msg;
  memcpy(msg->msg, stream, size);

  msg->args.args = NULL;
  msg->args.n_args = 0;
  msg->access = MSG_READ;
  msg->size = size;
  msg->i = 0;
  msg->it = msg->msg + strlen(msg->name) + sizeof(c8);

  return msg;

err_msg_msg:
  free(msg->name);
err_name:
  free(msg);
err_msg:
  WARN("could not create message '%s'", stream);
  return NULL;
};

size_t ui_msg_size(msg_T msg) {
  assert(msg && msg->name);

  if (msg->i < msg->args.n_args) {
    INFO("message %s, has not been processed yet.", msg->name);
    return 0;
  }
  return msg->size;
};

void *ui_msg_bs(msg_T msg) { return msg->msg; };

msg_T ui_msg_push_create(const c8 *name) {
  map_T map = ui_msg_browser_push_m();
  struct push_msg_bme *pmbme = (struct push_msg_bme *)gen_map_find(map, name);
  if (!pmbme) {
    WARN("'%s' is not registered.", name);
    return NULL;
  }

  msg_T msg = ui_msg_create(name, &pmbme->out);
  return msg;
};

const c8 *ui_msg_name(msg_T msg) { return msg->name; };

static s32 ui_args_check_(msg_T msg, enum ARG_TYPE t);

static s32 ui_args_check_(msg_T msg, enum ARG_TYPE t) {
  if (msg->i > msg->args.n_args) {
    WARN("No more arguments to read in %s", msg->msg);
    return 1;
  }
  if (t != msg->args.args[msg->i]) {
    WARN("Tried to read %s when next argument is expected to be %s from %s",
         ui_args_e2s_(t), msg->args.args[msg->i], msg->msg);
    return 1;
  }
  return 0;
};

s32 ui_msg_arg_read_s32(msg_T msg, s32 *val) {
  assert(msg);
  assert(msg->msg);
  if (ui_args_check_(msg, S32))
    return 1;

  memcpy(val, msg->it, sizeof(s32));
  msg->it += sizeof(s32);
  msg->i++;
  return 0;
};

s32 ui_msg_arg_read_u32(msg_T msg, u32 *val) {

  if (ui_args_check_(msg, U32))
    return 1;

  memcpy(val, msg->it, sizeof(u32));
  msg->it += sizeof(u32);
  msg->i++;
  return 0;
};

s32 ui_msg_arg_read_str(msg_T msg, c8 *val) {

  if (ui_args_check_(msg, STRING))
    return 1;

  size_t sl = ui_msg_str_size_r(msg) + 1;
  if (!sl)
    return 1;
  ui_msg_read_str_r(msg, val);
  msg->it += sizeof(c8) * sl;
  msg->i++;
  return 0;
};

size_t ui_msg_str_size(msg_T msg) {
  if (ui_args_check_(msg, STRING))
    return 0;

  // TODO: !important check buffer overflow
  return ui_msg_str_size_r(msg);
}

s32 ui_msg_write_str(msg_T msg, c8 *string) {
  if (msg->i > msg->args.n_args) {
    WARN("No more arguments to read in %s", msg->msg);
    return 1;
  }
  enum ARG_TYPE t = msg->args.args[msg->i];
  if (t != STRING) {
    WARN("Tried to read STRING when next argument is %s from %s",
         ui_args_e2s_(t), msg->msg);
    return 1;
  }
  ui_msg_read_str_r(msg, string);
  return 1;
};

void ui_msg_free(msg_T msg) {
  assert(msg);
  if (msg->args.args)
    free(msg->msg);
  if (msg->args.args)
    free(msg->args.args);
  free(msg->name);
  free(msg);
};

void ui_msg_cpy_name(msg_T msg, c8 *name) { strcpy(name, msg->name); };

void ui_msg_write_s32_r(msg_T msg, s32 val) {
  memcpy(msg->it, &val, sizeof(s32));
  msg->it += sizeof(s32);
  // msg->size += sizeof(s32);
};

void ui_msg_write_u32_r(msg_T msg, u32 val) {
  memcpy(msg->it, &val, sizeof(u32));
  msg->it += sizeof(u32);
};

void ui_msg_write_string_r(msg_T msg, const c8 *string) {

  size_t s_l = strlen(string);
  strcpy(msg->it, string);
  msg->it += sizeof(c8) * (1 + s_l);
};

void ui_msg_read_s32_r(msg_T msg, s32 *val) {
  memcpy(val, msg->it, sizeof(s32));
  msg->it += sizeof(s32);
};

void ui_msg_read_u32_r(msg_T msg, u32 *val) {
  memcpy(val, msg->it, sizeof(u32));
  msg->it += sizeof(u32);
};

// TOOD safely read string !IMPORTANT
void ui_msg_read_str_r(msg_T msg, c8 *string) {
  strcpy(string, msg->it);
  msg->it += sizeof(c8) * (1 + strlen(string));
};

size_t ui_msg_str_size_r(msg_T msg) { return strlen(msg->it); };
