#ifndef RENDER_H
#define RENDER_H

#include <dmath.h>
#include <types.h>

s32 ren_init();

s8 ren_draw_frame();

void ren_set_viewport(struct rect_T bound);

void ren_set_ui_background(s32 texture_id, struct rect_T bound);

void ren_update_ui_background_bitmap(u8 *bitmap);

void ren_update_ui_background(struct rect_T bound, u8 *bitmap);

void ren_set_gl_enabled(s32 enabled);

void ren_skybox_set(u32 top_hex, u32 bottom_hex, s32 stiffness);

#endif // !RENDER_H
