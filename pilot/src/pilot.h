#ifndef PILOT_H
#define PILOT_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "dmath.h"
#include "types.h"

enum pilot_ui_log_level {
  PILOT_UI_TRACE,
  PILOT_UI_DEBUG,
  PILOT_UI_INFO,
  PILOT_UI_WARN,
  PILOT_UI_ERROR
};

void pilot_ui_log(enum pilot_ui_log_level level, const c8 *fmt, ...);

void pilot_ui_log_flush(void);

void pilot_ui_cursor_clbk(s32 cursor_type);

s32 pilot_create_texture(const c8 *name, const char *file);

void pilot_set_msg_calls();

void pilot_ui_texture_clbk(u8 *bitmap, s32 isSubImage, struct rect_T rect);

void pilot_init_scene();

void pilot_create_targets();

void pilot_popup_show(s32 show);

void pilot_popup_size(s32 x, s32 y, s32 w, s32 h);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !PILOT_H
