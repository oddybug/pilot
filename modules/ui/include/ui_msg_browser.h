#ifndef UI_MSG_BROWSER
#define UI_MSG_BROWSER

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "ui_ipc.h"

extern msg_map_T ui_msg_pull_bm_e(struct map_it_T *it);

// extern void* ui_msg_pull_bm(size_t *size);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !UI_MSG_BROWSER
