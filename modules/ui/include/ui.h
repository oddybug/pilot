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

s32 ui_start(int argc, char *argv[]);

s32 ui_get_texture_id();

void ui_message_loop();

void ui_send_mouse_keydown(c16 key);

void ui_send_mouse_keyup(c16 key);

void ui_send_mouse_event_click(enum MOUSE_BTN mb, struct point_T m_p);

void ui_send_mouse_down(enum MOUSE_BTN mb, struct point_T m_p,
                               u32 modifiers);

void ui_send_mouse_up(enum MOUSE_BTN mb, struct point_T m_p,
                             u32 modifiers);

void ui_send_mouse_event_motion(struct point_T m_p, u32 modifiers);

bool ui_can_close();

void ui_close();

void ui_set_ui_texture_callback(void (*clbk)(u8 *buffer, u32 width,
                                                    u32 height));
enum UI_CURSOR {
  UI_CURSOR_POINTER = 0,
  UI_CURSOR_CROSS,
  UI_CURSOR_HAND,
  UI_CURSOR_IBEAM,
  UI_CURSOR_WAIT,
  UI_CURSOR_HELP,
  UI_CURSOR_EASTRESIZE,
  UI_CURSOR_NORTHRESIZE,
  UI_CURSOR_NORTHEASTRESIZE,
  UI_CURSOR_NORTHWESTRESIZE,
  UI_CURSOR_SOUTHRESIZE,
  UI_CURSOR_SOUTHEASTRESIZE,
  UI_CURSOR_SOUTHWESTRESIZE,
  UI_CURSOR_WESTRESIZE,
  UI_CURSOR_NORTHSOUTHRESIZE,
  UI_CURSOR_EASTWESTRESIZE,
  UI_CURSOR_NORTHEASTSOUTHWESTRESIZE,
  UI_CURSOR_NORTHWESTSOUTHEASTRESIZE,
  UI_CURSOR_COLUMNRESIZE,
  UI_CURSOR_ROWRESIZE,
  UI_CURSOR_MIDDLEPANNING,
  UI_CURSOR_EASTPANNING,
  UI_CURSOR_NORTHPANNING,
  UI_CURSOR_NORTHEASTPANNING,
  UI_CURSOR_NORTHWESTPANNING,
  UI_CURSOR_SOUTHPANNING,
  UI_CURSOR_SOUTHEASTPANNING,
  UI_CURSOR_SOUTHWESTPANNING,
  UI_CURSOR_WESTPANNING,
  UI_CURSOR_MOVE,
  UI_CURSOR_VERTICALTEXT,
  UI_CURSOR_CELL,
  UI_CURSOR_CONTEXTMENU,
  UI_CURSOR_ALIAS,
  UI_CURSOR_PROGRESS,
  UI_CURSOR_NODROP,
  UI_CURSOR_COPY,
  UI_CURSOR_NONE,
  UI_CURSOR_NOTALLOWED,
  UI_CURSOR_ZOOMIN,
  UI_CURSOR_ZOOMOUT,
  UI_CURSOR_GRAB,
  UI_CURSOR_GRABBING,
  UI_CURSOR_MIDDLE_PANNING_VERTICAL,
  UI_CURSOR_MIDDLE_PANNING_HORIZONTAL,
  UI_CURSOR_CUSTOM,
  UI_CURSOR_DND_NONE,
  UI_CURSOR_DND_MOVE,
  UI_CURSOR_DND_COPY,
  UI_CURSOR_DND_LINK,
  UI_CURSOR_NUM_VALUES,
};

void ui_set_cursor_callback(void (*clbk)(s32 cursor_type));
void ui_resize_window(u32 width, u32 height);

#ifdef __cplusplus
extern "C" {
#endif

map_T ui_msg_browser_push_m();

map_T ui_msg_browser_pull_m();

map_T ui_msg_render_pull_m();

s32 ui_msg_push_send(msg_T msg);

s32 ui_msg_push_listeners(const c8 *name);

#ifdef __cplusplus
}
#endif

void ui_close_browsers();

#endif // !UI_H
