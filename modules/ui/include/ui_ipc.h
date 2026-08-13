#ifndef UI_IPC_H
#define UI_IPC_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "types.h"

enum ARG_TYPE {
  U32 = 0,
  S32,
};

struct entry_T;

struct args_T {
  enum ARG_TYPE *args;
  u32 n_args;
};

extern void pilot_ipc_create_dicc();

extern s32 pilot_ipc_entry_create(c8 *name, void (*callback)(void *data),
                                  struct args_T *in_args,
                                  struct args_T *out_args);

extern void pilot_ipc_remove_entry(struct entry_T e);

extern void pilot_ipc_free_dicc();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !UI_IPC_H
