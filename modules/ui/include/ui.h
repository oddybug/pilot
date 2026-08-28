#ifndef UI_H
#define UI_H

#include "data/hashmap.h"
#include "dmath.h"
#include "types.h"

#include "ui_msg_common.h"

enum MOUSE_BTN {
  MBTN_LEFT = 0,
  MBTN_MIDDLE,
  MBTN_RIGHT,
};

extern s32 ui_start(int argc, char *argv[]);

extern s32 ui_get_texture_id();

extern void ui_message_loop();

extern void ui_send_mouse_keydown(c16 key);

extern void ui_send_mouse_keyup(c16 key);

extern void ui_send_mouse_event_click(enum MOUSE_BTN mb, struct point_T m_p);

extern void ui_send_mouse_event_motion(struct point_T m_p);

extern bool ui_can_close();

extern void ui_close();

extern void ui_set_ui_texture_callback(void (*clbk)(u8 *buffer, u32 width,
                                                    u32 height));
extern void ui_resize_window(u32 width, u32 height);

#ifdef __cplusplus
extern "C" {
#endif

map_T ui_msg_browser_push_m();

map_T ui_msg_browser_pull_m();

map_T ui_msg_render_pull_m();

extern s32 ui_msg_push_send(msg_T msg);
#ifdef __cplusplus
}
#endif

void ui_close_browsers();

#endif // !UI_H
