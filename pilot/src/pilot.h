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

s32 pilot_create_texture(const char *file);

void pilot_set_msg_calls();

void pilot_ui_texture_clbk(u8 *bitmap, u32 width, u32 height);

void pilot_init_scene();

void pilot_create_targets();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !PILOT_H
