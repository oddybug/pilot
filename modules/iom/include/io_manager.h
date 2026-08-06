#ifndef IO_MANAGER_H
#define IO_MANAGER_H

#include <SDL3/SDL_events.h>
#ifdef __cplusplus
extern "C" {
#endif

#include "dmath.h"
#include "types.h"

struct target_T {
  struct rect_T bounds;
  s32 id;
  s32 z;
  void (*iom_callback_fn)(SDL_Event *e);
};

#define MAX_TARGETS 8

static struct target_T targets[MAX_TARGETS];

extern s8 iom_init();

extern void iom_set_event_callback(void (*callback)(SDL_Event *e));

extern void iom_poll_events();

extern void iom_resize_window(u32 width, u32 height);

extern s32 iom_create_target();

extern void iom_set_target(s32 id, struct rect_T bounds, s32 z,
                          void (*iom_callback_fn)(SDL_Event *e));

extern s8 iom_quit();

#ifdef __cplusplus
}
#endif

#endif // !IO_MANAGER_H
