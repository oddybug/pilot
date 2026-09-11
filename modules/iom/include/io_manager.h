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
  u32 flags;
  void (*iom_callback_fn)(SDL_Event *e);
};

enum iom_target_flag {
  TARGET_CALLBACK_ALWAYS = 1u << 0,
};

#define MAX_TARGETS 8

static struct target_T targets[MAX_TARGETS];

extern s8 iom_init();

extern void iom_set_event_callback(void (*callback)(SDL_Event *e));

extern void iom_poll_events();

extern void iom_resize_target(s32 id, struct rect_T bounds);

extern s32 iom_create_target();

extern void iom_set_target(s32 id, struct rect_T bounds, s32 z,
                           void (*iom_callback_fn)(SDL_Event *e));

extern void iom_target_set_flag(s32 id, u32 flags);

extern void iom_target_clear_flag(s32 id, u32 flags);

extern s32 iom_routing_target(void);

/**
 * @brief returns widnows width and height
 *
 * @return point_T.x = width and point_T.y = height
 */
extern struct point_T iom_get_window_size();

/**
 * @brief Check if io manager can close correctly.
 *
 * @return 1 if can close and 0 otherwise.
 */
extern s32 iom_can_close();

enum IOM_CURSOR {
  IOM_CURSOR_DEFAULT = 0,
  IOM_CURSOR_TEXT,
  IOM_CURSOR_WAIT,
  IOM_CURSOR_CROSSHAIR,
  IOM_CURSOR_PROGRESS,
  IOM_CURSOR_NWSE_RESIZE,
  IOM_CURSOR_NESW_RESIZE,
  IOM_CURSOR_EW_RESIZE,
  IOM_CURSOR_NS_RESIZE,
  IOM_CURSOR_MOVE,
  IOM_CURSOR_NOT_ALLOWED,
  IOM_CURSOR_POINTER,
  IOM_CURSOR_NW_RESIZE,
  IOM_CURSOR_N_RESIZE,
  IOM_CURSOR_NE_RESIZE,
  IOM_CURSOR_E_RESIZE,
  IOM_CURSOR_SE_RESIZE,
  IOM_CURSOR_S_RESIZE,
  IOM_CURSOR_SW_RESIZE,
  IOM_CURSOR_W_RESIZE,
  IOM_CURSOR_COUNT,
};

extern void iom_set_cursor(s32 cursor);

extern s8 iom_quit();

#ifdef __cplusplus
}
#endif

#endif // !IO_MANAGER_H
