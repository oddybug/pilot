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

// struct msg_map {
//   c8 *name;
//   void *msg;
//   void *it;
//   // this could be simplified as now the message have lenght
//   size_t size;
// };

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

  if (msg->args.args)
    goto err_args;

  memcpy(msg->args.args, args, sizeof(enum ARG_TYPE) * args->n_args);
  msg->args.n_args = 0;
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
  default:
    return "[WRONG_TYPE_PROVIDED]";
  }
};

// DEPRECATED
/**
 * @brief Get the size in bytes of an arg
 *
 * @param type
 * @return
 */
// static size_t ui_args_arg_size_(enum ARG_TYPE type);

// THIS PROLLY REFACTORS TO A DATA TYPE args_size in arguments because string is
// undefined lenght
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

static size_t ui_args_argsv_get(struct args *args, va_list list);

static size_t ui_args_argsv_get(struct args *args, va_list list) {

  size_t msg_size = 0;
  for (int i = 0; i < args->n_args; i++) {
    switch (args->args[i]) {
    case U32:
      msg_size += ui_args_arg_size_(U32, va_arg(list, s32));
    case S32:
      msg_size += ui_args_arg_size_(S32, va_arg(list, s32));
    case ARG_TYPE:
      msg_size += ui_args_arg_size_(ARG_TYPE, va_arg(list, enum ARG_TYPE));
      break;
    case STRING:
      msg_size += ui_args_arg_size_(STRING, va_arg(list, c8 *));
      break;
    }
  }
  return msg_size;
};

// void ui_msg_map_push_s32(msg_map_T msg, s32 *val) {
//   memcpy(msg->it, val, sizeof(s32));
//   msg->it += sizeof(s32);
//   // msg->size += sizeof(s32);
// };
//
// void ui_msg_map_push_u32(msg_map_T msg, u32 *val) {
//   memcpy(msg->it, val, sizeof(u32));
//   msg->it += sizeof(u32);
//   // msg->size += sizeof(u32);
// };
//
// void ui_msg_map_read_s32(msg_map_T msg, s32 *val) {
//   memcpy(val, msg->it, sizeof(s32));
//   msg->it += sizeof(s32);
// };
//
// extern void ui_msg_map_read_u32(msg_map_T msg, u32 *val) {
//   memcpy(val, msg->it, sizeof(u32));
//   msg->it += sizeof(u32);
// };

// msg_map_T ui_msg_map_create(const c8 *name, struct args *in, struct args
// *out) {
//   assert(in || out || name);
//
//   msg_map_T msg = malloc(sizeof(struct msg_map));
//   if (!msg)
//     goto err;
//
//   msg->size = 0;
//   // msg->msg = malloc(msg_size);
//
//   // if (!msg->msg)
//   //   goto err_name;
//
//   // strcpy(msg->msg, name);
//   // msg->it = msg->msg + strlen(name) + sizeof(c8);
//
//   return msg;
// err_name:
//   free(msg);
// err:
//   return NULL;
// };

// void ui_msg_map_free(msg_map_T msg) {
//   assert(msg || msg->msg);
//   free(msg->msg);
//   free(msg);
// };

// void *ui_msg_map_bs(msg_map_T msg) {
//   assert(msg);
//   return msg->msg;
// };

// msg_map_T ui_msg_map_bs2m(void *stream, size_t size) {
//   msg_map_T msg = malloc(sizeof(struct msg_map));
//   if (!msg) {
//     goto err;
//   }
//   msg->msg = malloc(size);
//   if (!msg->msg) {
//     goto err_name;
//   }
//   mempcpy(msg->msg, stream, size);
//   msg->size = size;
//   msg->it = msg->msg + strlen(stream) + sizeof(c8);
//   return msg;
//
// err_name:
//   free(msg);
// err:
//   return NULL;
// };
//
// size_t ui_msg_map_size(msg_map_T msg) {
//   assert(msg || msg->msg);
//   return msg->size;
// };

static void ui_msg_fill_(msg_T msg, size_t args_s, va_list l) {
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
  msg->size = args_s;
err_fill:
  free(msg->msg);
err_name:
  return;
};

msg_T ui_msg_populate_(msg_T msg, ...) {
  assert(msg && msg->access == MSG_WRITE && msg->args.args);
  va_list list;
  va_start(list, msg);
  size_t args_s = ui_args_argsv_get(&msg->args, list);
  va_end(list);
  va_start(list, msg);
  ui_msg_fill_(msg, args_s, list);
  va_end(list);
  return msg;
};

// msg_T ui_msg_create_(const c8 *name, struct args *args) {
//   assert(args && name);
//   msg_T msg = malloc(sizeof(struct msg));
//   if (!msg)
//     goto err;
//
//   size_t msg_size = ui_args_argsv_get(args) + sizeof(name) + sizeof(c8);
//   msg->msg = malloc(msg_size);
//   if (!msg->msg)
//     goto err_name;
//   strcpy(msg->msg, name);
//
//   msg->it = msg->msg + strlen(name) + 1 * sizeof(c8);
//   msg->i = 0;
//   msg->args = args;
//
//   return msg;
// err_name:
//   free(msg);
// err:
//   return NULL;
// };

extern msg_T ui_msg_get_fs(void *stream, size_t stream_s, struct args *args) {
  msg_T msg = malloc(sizeof(struct msg));
  if (!msg)
    return NULL;
  msg->args = args;
  msg->msg = malloc(stream_s);
  if (!msg->msg) {
    free(msg);
    return NULL;
  }
  memcpy(msg->msg, stream, stream_s);
  msg->it = msg->msg + strlen(stream) + 1 * sizeof(c8);
  msg->i = 0;
  return msg;
};

extern msg_T ui_msg_get_fs_r(void *stream, size_t size) {
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
  msg->it = msg->msg + strlen(stream) + sizeof(c8);

  return msg;

err_msg_msg:
  free(msg->name);
err_name:
  free(msg);
err_msg:
  WARN("could not create message '%s'", stream);
  return NULL;
};

extern size_t ui_msg_size(msg_T msg) {
  assert(msg && msg->name);

  if (msg->i < msg->args->n_args) {
    INFO("message %s, has not been processed yet.", msg->name);
    return 0;
  }
  return msg->size;
};

extern void *ui_msg_bs(msg_T msg) { return msg->msg; };

extern msg_T ui_msg_push_create(const c8 *name) {
  map_T map = ui_msg_browser_push_m();
  struct push_msg_bme *pmbme = (struct push_msg_bme *)gen_map_find(map, name);
  if (!pmbme) {
    WARN("'%s' is not registered.", name);
    return NULL;
  }

  // msg_T msg = ui_msg_create_(name, &pmbme->out);
  // return msg;
};

extern const c8 *ui_msg_name(msg_T msg) { return msg->name; };

static s32 ui_args_check_(msg_T msg, enum ARG_TYPE t);

static s32 ui_args_check_(msg_T msg, enum ARG_TYPE t) {
  if (msg->i > msg->args.n_args) {
    WARN("No more arguments to read in %s", msg->msg);
    return 1;
  }
  if (t != S32) {
    WARN("Tried to read %s when next argument is expected to be %s from %s",
         ui_args_e2s_(t), msg->args.args[msg->i], msg->msg);
    return 1;
  }
  return 0;
};

s32 ui_msg_arg_read_s32(msg_T msg, s32 *val) {
  assert(msg);
  assert(msg->msg);
  if (!ui_args_check_(msg, S32))
    return 1;

  memcpy(val, msg->it, sizeof(s32));
  msg->it += sizeof(s32);
  msg->i++;
  return 0;
};

s32 ui_msg_arg_read_u32(msg_T msg, u32 *val) {

  if (!ui_args_check_(msg, U32))
    return 1;

  memcpy(val, msg->it, sizeof(u32));
  msg->it += sizeof(u32);
  msg->i++;
  return 0;
};

s32 ui_msg_str_size(msg_T msg) {
  if (!ui_args_check_(msg, STRING))
    return 1;

  // TODO: !important check buffer overflow
  return ui_msg_str_size_r(msg);
}

// s32 ui_msg_write_str(msg_T msg, c8 *string) {
//   if (msg->i > msg->args->n_args) {
//     WARN("No more arguments to read in %s", msg->msg);
//     return 1;
//   }
//   enum ARG_TYPE t = msg->args->args[msg->i];
//   if (t != STRING) {
//     WARN("Tried to read STRING when next argument is %s from %s",
//          ui_args_e2s_(t), msg->msg);
//     return 1;
//   }
//   ui_msg_read_str_r(msg, string);
//   return 1;
// };
//
// void ui_msg_arg_read(msg_T msg, void *val) {
//   switch (msg->args->args[msg->i]) {
//   S32:
//     {
//       ui_msg_arg_read_s32(msg, val);
//       break;
//     }
//   U32:
//     {
//       ui_msg_arg_read_u32(msg, val);
//       break;
//     }
//   ARG_TYPE:
//     break;
//   default:
//
//     break;
//   }
// };

extern void ui_msg_free(msg_T msg) {
  assert(msg);
  free(msg->msg);
  free(msg->args.args);
  free(msg->name);
  free(msg);
};

extern void ui_msg_cpy_name(msg_T msg, c8 *name) { strcpy(name, msg->msg); };

// s32 ui_msg_write_s32(msg_T msg, s32 val) {
//   if (msg->i > msg->args.n_args) {
//     WARN("No more arguments to push in %s", msg->msg);
//     return 1;
//   }
//
//   enum ARG_TYPE t = msg->args.args[msg->i];
//
//   if (t != S32) {
//     WARN("Tried to push S32 when next argument is %s from %s",
//     ui_args_e2s_(t),
//          msg->msg);
//     return 1;
//   }
//
//   memcpy(msg->it, &val, sizeof(s32));
//
//   msg->it += sizeof(s32);
//   msg->i++;
//   return 0;
// };
//
// extern s32 ui_msg_write_u32(msg_T msg, u32 val) {
//   if (msg->i > msg->args->n_args) {
//     WARN("No more arguments to push in %s", msg->msg);
//     return 1;
//   }
//   enum ARG_TYPE t = msg->args->args[msg->i];
//   if (t != U32) {
//     WARN("Tried to push U32 when next argument is %s from %s",
//     ui_args_e2s_(t),
//          msg->msg);
//     return 1;
//   }
//   memcpy(msg->it, &val, sizeof(u32));
//
//   msg->it += sizeof(u32);
//   msg->i++;
//   return 0;
// };

// extern s32 ui_msg_write_string(msg_T msg, const c8 *string) {
//   if (!ui_args_check_(msg, STRING))
//     return 1;
//   ui_msg_write_string_r(msg, string);
// };

// void ui_msg_write_s32_r(msg_T msg, s32 val) {
//   memcpy(msg->it, &val, sizeof(s32));
//   msg->it += sizeof(s32);
//   // msg->size += sizeof(s32);
// };
//
// void ui_msg_write_u32_r(msg_T msg, u32 val) {
//   memcpy(msg->it, &val, sizeof(u32));
//   msg->it += sizeof(u32);
// };
//
// extern void ui_msg_write_string_r(msg_T msg, const c8 *string) {
//
//   size_t s_l = strlen(string);
//   strcpy(msg->it, string);
//   msg->it += sizeof(c8) * (2 + s_l);
// };
//
// void ui_msg_read_s32_r(msg_T msg, s32 *val) {
//   memcpy(val, msg->it, sizeof(s32));
//   msg->it += sizeof(s32);
// };
//
// void ui_msg_read_u32_r(msg_T msg, u32 *val) {
//   memcpy(val, msg->it, sizeof(u32));
//   msg->it += sizeof(u32);
// };

// TOOD safely read string !IMPORTANT
void ui_msg_read_str_r(msg_T msg, c8 *string) {
  strcpy(string, msg->it);
  msg->it += sizeof(c8) * (1 + strlen(string));
};

size_t ui_msg_str_size_r(msg_T msg) { return strlen(msg->it); };
