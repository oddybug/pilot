#ifndef UI_MSG_COMMON_H
#define UI_MSG_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "data/hashmap.h"
#include "data/list.h"
#include "types.h"

#define DICC_SIZE 1024

enum ARG_TYPE { U32 = 0, S32, STRING, ARG_TYPE };

enum MSG_ACCESS { MSG_READ = 0, MSG_WRITE };

struct args {
  enum ARG_TYPE *args;
  u32 n_args;
};

typedef struct msg *msg_T;

msg_T ui_msg_create(const c8 *name, struct args *args);

msg_T ui_msg_get_fs(void *stream, size_t stream_s, struct args *args);

msg_T ui_msg_get_fs_r(void *stream, size_t size);

#define ui_msg_populate(msg, ...) ui_msg_populate_(msg, __VA_ARGS__, NULL)

void ui_msg_populate_(msg_T msg, ...);

void ui_msg_populate_r(msg_T msg, list_T list);

void ui_msg_free(msg_T msg);

size_t ui_msg_size(msg_T msg);

extern void *ui_msg_bs(msg_T msg);

msg_T ui_msg_push_create(const c8 *name);

void ui_msg_cpy_name(msg_T msg, c8 *name);

extern const c8 *ui_msg_name(msg_T msg);

// access == WRITE

// access == READ
s32 ui_msg_arg_read_s32(msg_T msg, s32 *val);

s32 ui_msg_arg_read_u32(msg_T msg, u32 *val);

s32 ui_msg_arg_read_str(msg_T msg, c8 *val);

size_t ui_msg_str_size(msg_T msg);

// access == WRITE
s32 ui_msg_write_str(msg_T msg, c8 *string);

void ui_msg_write_s32_r(msg_T msg, s32 val);

void ui_msg_write_u32_r(msg_T msg, u32 val);

void ui_msg_write_string_r(msg_T msg, const c8 *string);

// access == READ

void ui_msg_read_s32_r(msg_T msg, s32 *val);

void ui_msg_read_u32_r(msg_T msg, u32 *val);

void ui_msg_read_str_r(msg_T msg, c8 *string);

size_t ui_msg_str_size_r(msg_T msg);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !UI_MSG_COMMON_H
