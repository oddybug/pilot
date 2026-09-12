#ifndef GLOBAL_H
#define GLOBAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "dmath.h"
#include "types.h"

#define PILOT_WIN_DEFAULT_W 1280
#define PILOT_WIN_DEFAULT_H 720

struct pilot_cfg_T {
  s32 win_w;
  s32 win_h;
  struct rect_T ui_bounds;
  s32 cef_texture;
  struct rect_T gl_bounds;
  s32 gl_ready;
};

extern struct pilot_cfg_T g_cfg;

extern s32 g_hud_m_id;

extern s32 g_gl_target;
extern s32 g_ui_target;

void pilot_cfg_init(void);

#ifdef __cplusplus
}
#endif

#endif
