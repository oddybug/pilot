#ifndef RENDER_H
#define RENDER_H

#include <dmath.h>
#include <types.h>

extern s32 ren_init();

extern s8 ren_draw_frame();

extern void ren_set_viewport(struct rect_T bound);

extern void ren_set_ui_background(s32 texture_id, struct rect_T bound);

extern void ren_update_ui_background_bitmap(u8 *bitmap);

extern void ren_update_ui_background(struct rect_T bound, u8 *bitmap);

#endif // !RENDER_H
