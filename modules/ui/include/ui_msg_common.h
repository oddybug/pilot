#ifndef UI_MSG_COMMON_H
#define UI_MSG_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "data/hashmap.h"
#include "types.h"

#define DICC_SIZE 1024

enum ARG_TYPE { U32 = 0, S32, STRING, ARG_TYPE };

struct response_T {
  const c8 *key;
  struct args *args;
  void *const msg;
  void *it;
};

struct args {
  enum ARG_TYPE *args;
  u32 n_args;
};

typedef struct msg *msg_T;
typedef struct msg_map *msg_map_T;

// Render process

extern void ui_msg_map_push_s32(msg_map_T msg, s32 *val);

extern void ui_msg_map_push_u32(msg_map_T msg, u32 *val);

extern void ui_msg_map_read_s32(msg_map_T msg, s32 *val);

extern void ui_msg_map_read_u32(msg_map_T msg, u32 *val);

// extern void ui_msg_map_push_u32(msg_map_T msg, u32 *val);

extern msg_map_T ui_msg_map_create(const c8 *name, struct args *in,
                                   struct args *out);

extern void ui_msg_map_free(msg_map_T msg);

extern void *ui_msg_map_bs(msg_map_T msg);

extern msg_map_T ui_msg_map_bs2m(void *stream, size_t size);

extern size_t ui_msg_map_size(msg_map_T msg);

extern msg_T ui_msg_create_(const c8 *name, struct args *args);

/**
 * @brief Get message from stream. It copies stream into a new buffer.
 * Developer is in charge to handle stream allocation.
 *
 * @param stream
 * @param args args of the message
 * @return NULL if msg could not get created memory address of msg_T otherwise
 */
extern msg_T ui_msg_get_fs(void *stream, struct args *args);

extern msg_T ui_msg_get_fs_raw(void *stream, size_t size);

extern size_t ui_msg_size(msg_T msg);

extern void *ui_msg_bitstream(msg_T msg);

extern msg_T ui_msg_push_create(const c8 *name);

extern void ui_msg_free(msg_T msg);

extern void ui_msg_cpy_name(msg_T msg, c8 *name);

extern s32 ui_msg_push_s32(msg_T msg, s32 val);

extern s32 ui_msg_push_u32(msg_T msg, u32 val);

extern s32 ui_msg_arg_push_string(msg_T msg, const c8 *string);

extern s32 ui_msg_arg_read_s32(msg_T msg, s32 *val);

extern s32 ui_msg_arg_read_u32(msg_T msg, u32 *val);

extern s32 ui_msg_string_size(msg_T msg);

extern s32 ui_msg_read_string(msg_T msg, c8 *string);

extern void ui_msg_push_s32_r(msg_T msg, s32 val);

extern void ui_msg_push_u32_r(msg_T msg, u32 val);

extern void ui_msg_arg_push_string_r(msg_T msg, const c8 *string);

extern void ui_msg_read_s32_r(msg_T msg, s32 *val);

extern void ui_msg_read_u32_r(msg_T msg, u32 *val);

void ui_msg_read_string_r(msg_T msg, c8 *string);

size_t ui_msg_string_size_r(msg_T msg);

/**
 * @brief INTERNAL CALL! Get the size in bytes of the args vector in an
 * astgs_T struct
 *
 * @param args
 * @return
 */
extern size_t ui_args_argsv_get(struct args *args);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !UI_MSG_COMMON_H
