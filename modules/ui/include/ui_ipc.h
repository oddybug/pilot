#ifndef UI_IPC_H
#define UI_IPC_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "types.h"

#include "data/hashmap.h"

#define DICC_SIZE 1024

enum ARG_TYPE {
  U32 = 0,
  S32,
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
  void (*callback)(void *data);
  struct entry_T e;
};

extern s32 pilot_ipc_entry_c_create(struct map_T *map, const c8 *name,
                                    void (*callback)(void *data),
                                    struct args_T *in_args,
                                    struct args_T *out_args);

extern void pilot_ipc_remove_entry(struct entry_T e);

extern void pilot_ipc_free_dicc();

// wallahi this needs to be renamed and explained well because even I 5 min
// after doing it know nothing
extern u32 pilot_ipc_dicc_get_size(struct map_T *map, u32 *n_entries);

extern void *pilot_ipc_get_args_bs(struct map_T *map);

extern void pilot_ipc_stream_insert(struct map_T *map, void *stream);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !UI_IPC_H
