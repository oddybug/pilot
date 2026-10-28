#ifndef UI_ACCELERATED_H
#define UI_ACCELERATED_H

#include "dmath.h"
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PILOT_DMABUF_MAX_PLANES 4

// Plain-C mirror of CefAcceleratedPaintInfo (dmabuf planes + modifier).
// Kept free of CEF headers so SDL/render code can consume it.
struct pilot_dmabuf_plane_T {
  u32 stride;
  u64 offset;
  u64 size;
  s32 fd;
};

struct pilot_dmabuf_frame_T {
  struct pilot_dmabuf_plane_T planes[PILOT_DMABUF_MAX_PLANES];
  s32 plane_count;
  u64 modifier;
  s32 format; // cef_color_type_t value, opaque here
  s32 width;
  s32 height;
};

typedef void (*PilotAcceleratedFrameFn)(const struct pilot_dmabuf_frame_T *frame,
                                        s32 is_popup,
                                        struct rect_T popup_rect);

#ifdef __cplusplus
}
#endif

#endif // UI_ACCELERATED_H
