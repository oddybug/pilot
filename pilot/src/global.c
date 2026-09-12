#include "global.h"

struct pilot_cfg_T g_cfg;

s32 g_hud_m_id;

s32 g_gl_target;
s32 g_ui_target;

void pilot_cfg_init(void) {
  g_cfg.win_w = PILOT_WIN_DEFAULT_W;
  g_cfg.win_h = PILOT_WIN_DEFAULT_H;
  g_cfg.ui_bounds.x = 0;
  g_cfg.ui_bounds.y = 0;
  g_cfg.ui_bounds.w = PILOT_WIN_DEFAULT_W;
  g_cfg.ui_bounds.h = PILOT_WIN_DEFAULT_H;
  g_cfg.cef_texture = 0;
  g_cfg.gl_bounds.x = 0;
  g_cfg.gl_bounds.y = 0;
  g_cfg.gl_bounds.w = 0;
  g_cfg.gl_bounds.h = 0;
  g_cfg.gl_ready = 0;
}
