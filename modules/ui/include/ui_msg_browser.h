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
  // struct push_msg_e_render e;
};

struct push_msg_bme {
  // i need the fucking linked list here
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

// extern msg_map_T ui_msg_push_bm_e(struct map_it_T *it);

// extern msg_T ui_msg_pull_create(const c8 *name);

// extern msg_T ui_msg_push_create(const c8 *name);

// extern void* ui_msg_pull_bm(size_t *size);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !UI_MSG_BROWSER
