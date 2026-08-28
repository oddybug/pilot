#ifndef UI_MSG_RENDER_H
#define UI_MSG_RENDER_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "ui_msg_common.h"

struct pull_msg_e_render {
  struct args in;
  struct args out;
};

struct push_msg_e_render {
  struct args out;
};

extern void ui_msg_pushme_free(struct push_msg_e_render *e);

extern void ui_msg_pullme_free(struct pull_msg_e_render *e);

s32 ui_msg_pull_rm_add(void *stream, size_t size);

msg_T ui_msg_pull_render_create(c8 *name);

// msg_T ui_msg_push_render_create(c8 *name);

msg_T ui_msg_push_request(const c8 *name, struct args *args);

extern void ui_msg_arg_read(msg_T msg, void *val);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !UI_MSG_RENDER_H
