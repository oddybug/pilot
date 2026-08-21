#ifndef UI_MSG_RENDER_H
#define UI_MSG_RENDER_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "ui_ipc.h"

struct pull_msg_e_render {
  struct args in;
  struct args out;
};

extern void ui_msg_pullme_free(struct pull_msg_e_render *e);

s32 ui_msg_pull_rm_add(void *stream, size_t size);

msg_T ui_msg_pull_render_create(c8 *name);

//extern struct map_T *ui_msg_render_pull_m();

extern void *ui_msg_arg_read(msg_T msg);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !UI_MSG_RENDER_H
