#ifndef UI_MSG_BROWSER
#define UI_MSG_BROWSER

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "data/list.h"
#include "ui_msg_common.h"

struct pull_msg_bme {
  void (*callback)(msg_T msg, msg_T response);
  struct args in;
  struct args out;
};

struct push_msg_bme {
  list_T render;
  struct args out;
};

extern void ui_msg_pullem_free_clbk_(struct item_T *item);

extern void ui_msg_pullem_free(struct pull_msg_bme *e);

extern void ui_msg_pushem_free_clbk_(struct item_T *item);

extern void ui_msg_pushem_free(struct push_msg_bme *e);

extern s32 ui_msg_pull_new_entry(const c8 *name,
                                 void (*callback)(msg_T msg, msg_T response),
                                 struct args *in, struct args *out);

extern s32 ui_msg_push_new_entry(const c8 *name, struct args *out);

extern msg_T ui_msg_pull_bm_e(struct map_it_T *it);

extern s32 ui_msg_pull_new(const c8 *name,
                           void (*callback)(msg_T msg, msg_T response));

#define ui_msg_pull_set_i(name, ...)                                           \
  ui_msg_pull_set_i_h((name), __VA_ARGS__ __VA_OPT__(, ) ARG_END)

extern s32 ui_msg_pull_set_i_h(const c8 *name, ...);

#define ui_msg_pull_set_o(name, ...)                                           \
  ui_msg_pull_set_o_h((name), __VA_ARGS__ __VA_OPT__(, ) ARG_END)

extern s32 ui_msg_pull_set_o_h(const c8 *name, ...);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !UI_MSG_BROWSER
