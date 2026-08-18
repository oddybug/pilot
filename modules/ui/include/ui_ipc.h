#ifndef UI_IPC_H
#define UI_IPC_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "data/hashmap.h"
#include "types.h"

#define DICC_SIZE 1024

enum ARG_TYPE {
  U32 = 0,
  S32,
};

struct response_T {
  const c8 *key;
  struct args_T *args;
  void *const msg;
  void *it;
};

struct args_T {
  enum ARG_TYPE *args;
  u32 n_args;
};

// Browser process
struct entry_T {
  struct args_T in_args;
  struct args_T out_args;
};

// Render process
struct entry_c_T {
  void (*callback)(void *data, struct response_T *response);
  struct entry_T e;
};

extern s32 ui_ipc_entry_add(const c8 *name,
                            void (*callback)(void *data,
                                             struct response_T *response),
                            struct args_T *in_args, struct args_T *out_args);

// TODO: To implement
extern void ui_ipc_entry_rm(struct entry_T e);

extern void ui_ipc_free();

/**
 * @brief INTERNAL CALL! Get the size in bytes of the args vector in an astgs_T
 * struct
 *
 * @param args
 * @return
 */
extern size_t ui_ipc_argsv_get(struct args_T *args);

/**
 * @brief INTERNAL CALL! Gets the size in bytes of the total space needed for
 * allocate the bitstream of the map of entries to send it to the render process
 * (CEF).
 *
 * @param n_entries set pointer value to the total number of entries of the map
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
extern void *ui_ipc_stream_get();

extern struct map_T *ui_ipc_get_browser_map();

/**
 * @brief INTERNAL CALL! inserts entry into the render process map
 *
 * Implementation in ui_ipc.c
 *
 * @param map
 * @param stream
 */
extern void ui_ipc_stream_insert(struct map_T *map, void *stream);

/**
 * @brief copy value of type ARG_TYPE into stream and moves the pointer of the
 * stream to the las copied byte plus one.
 *
 * @param stream
 * @param value
 * @param type
 */
extern void ui_ipc_stream_write_arg(void **stream, void *value,
                                    enum ARG_TYPE type);

extern void ui_ipc_stream_write_string(void **stream, const c8 *string);

/**
 * @brief read the next value from stream and copies it to value arg. Stream
 * moves to the last copied byte plus onStream moves to the last copied byte
 * plus one.
 *
 * @param stream
 * @param value
 * @param type
 */
extern void ui_ipc_stream_read_arg(void **stream, void *value,
                                   enum ARG_TYPE type);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !UI_IPC_H
