
#include "SDL3/SDL_error.h"
#include "SDL3/SDL_video.h"
#include "glad/egl.h"
#include "glad/gl.h"

#include <stdio.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "io_manager.h"

#define OPENGL_MAJOR 4
#define OPENGL_MINOR 6

#define WINDOW_NAME "PILOT"

#define DEFAULT_SCR_WIDTH 800
#define DEFAULT_SCR_HEIGHT 600

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

extern s8 iom_quit() {
  SDL_GL_DestroyContext(gl_context);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}

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

  //// TODO: user need to have control over the main loop
  // while (!done) {
  //   SDL_Event event;
  //   while (SDL_PollEvent(&event)) {
  //     if (event.type == SDL_EVENT_QUIT) {
  //       done = true;
  //     }
  //   }

  //  // Render loop
  //}

  // iom_quit();
  return 0;
}

static void (*_iom_event_callback)(SDL_Event *e);

extern void iom_set_event_callback(void (*callback)(SDL_Event *e)) {
  _iom_event_callback = callback;
};

extern void iom_poll_events() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    _iom_event_callback(&event);
  }


  SDL_GL_SwapWindow(window);
};
