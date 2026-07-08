#ifndef IO_MANAGER_H
#define IO_MANAGER_H

#include <SDL3/SDL_events.h>
#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

extern s8 iom_init();

extern void iom_set_event_callback(void (*callback)(SDL_Event *e));

extern void iom_poll_events();

extern s8 iom_quit();

#ifdef __cplusplus
}
#endif

#endif // !IO_MANAGER_H
