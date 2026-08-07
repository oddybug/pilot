
#include "SDL3/SDL_error.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_keycode.h"
#include "SDL3/SDL_video.h"
#include "glad/egl.h"
#include "glad/gl.h"

#include <assert.h>
#include <stdio.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "io_manager.h"

#include "data/serial.h"
#include "log.h"

static struct serial_T *serial;

#define OPENGL_MAJOR 4
#define OPENGL_MINOR 6

#define WINDOW_NAME "PILOT"

#define DEFAULT_SCR_WIDTH 1280
#define DEFAULT_SCR_HEIGHT 720

static s32 g_running;

static struct point_T current_mouse_pos_;
static s32 current_target_;

/*
 * SDL
 */

SDL_Window *window = NULL;
SDL_GLContext gl_context = NULL;

/**
 * @brief initialize SDL context
 *
 * @return returns 0 on succes.
 */
s8 iom_init_sdl() {

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not initialize SDL: %s\n",
                 SDL_GetError());
    return 1;
  }

  g_running = 1;

  SDL_SetHint(SDL_HINT_VIDEO_FORCE_EGL, "1");
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, OPENGL_MAJOR);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, OPENGL_MINOR);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  return 0;
};

/**
 * @brief creates a sdl window
 *
 * @param title
 * @param width
 * @param height
 * @return returns 0 on succes
 */
s8 iom_create_window(const char *title, u32 width, u32 height);

s8 iom_create_window(const char *title, u32 width, u32 height) {

  window = SDL_CreateWindow(title, width, height, SDL_WINDOW_OPENGL);
  if (window == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create window: %s\n",
                 SDL_GetError());
    SDL_Quit();
    return 1;
  }

  gl_context = SDL_GL_CreateContext(window);
  if (gl_context == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create GL context: %s\n",
                 SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  return 0;
}

/**
 * @brief Creates a sdl default window with WINDOW_NAME, DEFAULT_SCR_WIDTH,
 * DEFAULT_SCR_HEIGHT defines.
 *
 * @return returns 0 on succes
 */
static s8 iom_create_default_window(void);

static s8 iom_create_default_window(void) {
  return iom_create_window(WINDOW_NAME, DEFAULT_SCR_WIDTH, DEFAULT_SCR_HEIGHT);
};

/**
 * @brief initialize OpenGL context
 *
 * @return returns 0 on succes.
 */
static s8 iom_init_glad_gl();

static s8 iom_init_glad_gl() {

  int version = gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress);

  if (version == 0) {
    return 1;
  }

  return 0;
};

static EGLDisplay egl_display;

/**
 * @brief initialize OpenGL ES context
 *
 * @return returns 0 on succes.
 */
static s8 iom_init_glad_gles(void);

static s8 iom_init_glad_gles(void) {
  egl_display = (EGLDisplay)SDL_EGL_GetCurrentDisplay();

  if (egl_display == NULL) {
    fprintf(stderr, "%s", SDL_GetError());
  }

  if (!gladLoadEGL(egl_display, (GLADloadfunc)SDL_GL_GetProcAddress)) {
    fprintf(stderr, "Failed to load GLAD EGL symbols\n");
    return -1;
  };
  return 0;
};

extern s8 iom_init() {

  bool done = false;

  s8 err = iom_init_sdl();

  if (err != 0) {
    fprintf(stderr, "SDL exited with error code %d\n", err);
  }

  err = iom_create_default_window();

  if (err != 0) {
    fprintf(stderr,
            "SDL exited with error code %d\n when creating default window",
            err);
  }

  err = iom_init_glad_gl();

  if (err != 0) {
    fprintf(stderr, "Glad exited with error code %d\n while loading GL", err);
  }

  err = iom_init_glad_gles();

  if (err != 0) {
    fprintf(stderr, "Glad exited with error code %d\n while loading GL ES",
            err);
  }

  return 0;
}

static void (*_iom_event_callback)(SDL_Event *e);

extern void iom_set_event_callback(void (*callback)(SDL_Event *e)) {
  _iom_event_callback = callback;
};

/**
 * @brief check if p is inside of r in a plain
 *
 * @param r
 * @param p
 * @return 1 if true 0 if false
 */

static s32 iom_rect_point_collision_(struct rect_T r, struct point_T p);

static s32 iom_rect_point_collision_(struct rect_T r, struct point_T p) {
  return (p.x > r.x && p.y > r.y && p.x < r.x + r.w && p.y < r.y + r.h);
}

static s32 iom_select_target_(struct point_T mouse_pos);

static s32 iom_select_target_(struct point_T mouse_pos) {

  // Cost: O(n): Is ts horrible? Yes. Can it be done in O(log n) or less?
  // Absolutely. Do I care? No. Do I have skill isues? Most likely. I mean
  // have you read the rest of the code already? What did you expect young
  // fella?

  s32 id;
  s32 max_z = -1;
  s32 t_collided = 0;
  s32 same_z = 0;

  for (id = 0; id < MAX_TARGETS; id++) {
    struct target_T t = targets[id];
    if (t.id == 0)
      continue;
    if (!iom_rect_point_collision_(t.bounds, mouse_pos))
      continue;
    if (t.z > max_z) {
      max_z = t.z;
      t_collided = id;
      same_z = 0;
    } else if (t.z == max_z) {
      same_z = 1;
    }
  }

  if (same_z) {
    WARN("two targets with the same z overlaing. Mouse and keyboard actions "
         "can have undefined behaivour.");
  }

  current_target_ = t_collided;
  return t_collided;

  return -1;
};

static void iom_alert_all_targets_(SDL_Event *event);

static void iom_alert_all_targets_(SDL_Event *event) {
  s32 id;
  for (id = 0; id < MAX_TARGETS; id++) {
    if (targets[id].id == 0)
      continue;
    targets[id].iom_callback_fn(event);
  }
}

extern void iom_poll_events() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {

    switch (event.type) {
    case SDL_EVENT_WINDOW_RESIZED:
      iom_alert_all_targets_(&event);
      break;
    case SDL_EVENT_MOUSE_MOTION: {
      struct point_T mouse_p = {.x = event.motion.x, .y = event.motion.y};
      (void)iom_select_target_(mouse_p);
    }
      [[fallthrough]];
    case SDL_EVENT_KEY_DOWN:
      if (event.key.key == SDLK_ESCAPE) {
        g_running = 0;
      }
      [[fallthrough]];
    default:
      if (current_target_ == 0) {
        WARN("No target selected. Mouse and keyboard undefined behaivour.");
      } else {
        targets[current_target_].iom_callback_fn(&event);
      }
      break;
    }
  }

  SDL_GL_SwapWindow(window);
};

void iom_resize_target(s32 id, struct rect_T bounds) {
  assert(targets[id].id != 0);
  targets[id].bounds = bounds;
};

s32 iom_create_target() {
  if (serial == NULL) {
    serial = gen_serial_create_from(1);
  }

  assert(serial != NULL);

  s32 id;
  if (gen_serial_stamp(serial, &id) != 0)
    return -1;

  return id;
}

void iom_set_target(s32 id, struct rect_T bounds, s32 z,
                    void (*iom_callback_fn)(SDL_Event *e)) {
  targets[id].id = id;
  targets[id].bounds = bounds;
  targets[id].z = z;
  targets[id].iom_callback_fn = iom_callback_fn;
};

extern struct point_T iom_get_window_size() {
  s32 w, h = 0;
  SDL_GetWindowSize(window, &w, &h);
  struct point_T res = {.x = w, .y = h};
  return res;
};

extern s32 iom_can_close() { return !g_running; };

extern s8 iom_quit() {
  SDL_GL_DestroyContext(gl_context);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
