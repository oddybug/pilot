#ifndef UI_IPC_H
#define UI_IPC_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "data/hashmap.h"
#include "types.h"

#define DICC_SIZE 1024

enum ARG_TYPE { U32 = 0, S32, ARG_TYPE };

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
struct pull_msg_e {
  void (*callback)(msg_T msg, msg_T response);
  struct args in;
  struct args out;
  // struct push_msg_e_render e;
};

extern void ui_msg_map_push_s32(msg_map_T msg, s32 *val);

extern void ui_msg_map_push_u32(msg_map_T msg, u32 *val);

extern void ui_msg_map_read_s32(msg_map_T msg, s32 *val);

extern void ui_msg_map_read_u32(msg_map_T msg, u32 *val);

extern msg_map_T ui_msg_map_create(const c8 *name, struct args *in,
                                   struct args *out);

extern void ui_msg_map_free(msg_map_T msg);

extern void *ui_msg_map_bs(msg_map_T msg);

extern msg_map_T ui_msg_map_bs2m(void *stream, size_t size);

extern size_t ui_msg_map_size(msg_map_T msg);

// TODO: implement
extern s32 ui_msg_push_send(msg_T msg);

extern s32 ui_msg_pull_new_entry(const c8 *name,
                                 void (*callback)(msg_T msg, msg_T response),
                                 struct args *in, struct args *out);

extern msg_T ui_msg_create_(const c8 *name, struct args *args);

extern s32 ui_msg_push_new_entry();

/**
 * @brief Get message from stream. It copies stream into a new buffer.
 * Developer is in charge to handle stream allocation.
 *
 * @param stream
 * @param args args of the message
 * @return NULL if msg could not get created memory address of msg_T otherwise
 */
extern msg_T ui_msg_get_fs(void *stream, struct args *args);

extern size_t ui_msg_size(msg_T msg);

extern void* ui_msg_bitstream(msg_T msg);

extern msg_T ui_msg_pull_create(const c8 *name);

extern msg_T ui_msg_push_create(const c8 *name);

extern void ui_msg_free(msg_T msg);

extern s32 ui_msg_push_s32(msg_T msg, s32 val);

extern s32 ui_msg_push_u32(msg_T msg, u32 val);

extern s32 ui_msg_arg_push_string(msg_T msg, const c8 *string);

extern s32 ui_msg_arg_read_s32(msg_T msg, s32 *val);

extern s32 ui_msg_arg_read_u32(msg_T msg, u32 *val);

//extern void* ui_msg_arg_read(msg_T msg);

extern s32 ui_ipc_entry_add(const c8 *name,
                            void (*callback)(void *data,
                                             struct response_T *response),
                            struct args *in_args, struct args *out_args);

extern struct map_T *ui_msg_render_pull_m();

// extern s32 ui_msg_pull_send();

// TODO: To implement
// extern void ui_ipc_entry_rm(struct push_msg_e_render e);

extern void ui_ipc_free();

/**
 * @brief INTERNAL CALL! Get the size in bytes of the args vector in an
 * astgs_T struct
 *
 * @param args
 * @return
 */
extern size_t ui_args_argsv_get(struct args *args);

/**
 * @brief INTERNAL CALL! Gets the size in bytes of the total space needed for
 * allocate the bitstream of the map of entries to send it to the render
 * process (CEF).
 *
 * @param n_entries set pointer value to the total number of entries of the
 * map
 * @return
 */
extern u32 ui_ipc_stream_get_size(u32 *n_entries);

// To remove prolly
extern void temp_print_buffer(void *stream, const u32 size);

/**
 * @brief get the bitstream of the entry dicc to pass it to the renderer
 *
 * BITSTREAM: bytes - content
 * header: u32 - num of entries
 * entry: strlen - entry key name
 *        u32 - num of inputs args
 *        enum ARG_TYPES * n_in - array of in ARG_TYPES
 *        u32 - num of outputs args
 *        enum ARG_TYPES * n_out - array of out ARG_TYPES
 *
 * @return a void pointer to the allocated data
 */
// DEPRECATED
// extern void *ui_ipc_stream_get();

extern struct map_T *ui_msg_browser_pull_m();

/**
 * @brief INTERNAL CALL! inserts entry into the render process map
 *
 * Implementation in ui_ipc.c
 *
 * @param map
 * @param stream
 */
extern void ui_ipc_stream_insert(struct map_T *map, void *stream);

// DEPRECATED
/**
 * @brief copy value of type ARG_TYPE into stream and moves the pointer of the
 * stream to the las copied byte plus one.
 *
 * @param stream
 * @param value
 * @param type
 */
// extern void ui_ipc_stream_write_arg(void **stream, void *value,
//                                     enum ARG_TYPE type);

// DEPRECATED
// extern void ui_ipc_stream_write_string(void **stream, const c8 *string);

// DEPRECATED
/**
 * @brief read the next value from stream and copies it to value arg. Stream
 * moves to the last copied byte plus onStream moves to the last copied byte
 * plus one.
 *
 * @param stream
 * @param value
 * @param type
 */
// extern void ui_ipc_stream_read_arg(void **stream, void *value,
//                                    enum ARG_TYPE type);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !UI_IPC_H
