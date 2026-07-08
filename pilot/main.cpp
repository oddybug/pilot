
// #include "object.h"

#include "SDL3/SDL_events.h"
#include "entity.h"
// #include "render.h"
//  #include "shader.h"
#include "log.h"
#include "object.h"

extern "C" {

#include "io_manager.h"
#include "render.h"
#include "shader.h"
//  #include "log.h"
//  #include "object.h"
}

// #include "render.h"

// int init_egl() {
//   if (!gladLoadEGL(demo_profile.egl_display,
//                    (GLADloadfunc)SDL_GL_GetProcAddress)) {
//     std::cerr << "Failed to load GLAD EGL symbols" << std::endl;
//     return -1;
//   }
//
//   EGLint major, minor;
//   if (!eglInitialize(demo_profile.egl_display, &major, &minor)) {
//     error("Failed to initialize EGL");
//   }
//
//   info("EGL Initialized: ", major, ".", minor);
//   info("EGL display: ", demo_profile.display);
//
//   return 0;
// };

#include "camera.h"

static s32 enable;

void sdl_callback(SDL_Event *e) {
  switch (e->type) {
  case SDL_EVENT_QUIT:
    enable = 1;
    break;
  case SDL_EVENT_KEY_DOWN:
    switch (e->key.key) {
    case SDLK_ESCAPE:
      enable = 1;
      break;
    }
    break;
  default:
    break;
  }
}

void start_camera() {
  ren_set_camera_aspect_ratio_wh(800, 600);
  ren_set_camera_planes(0.1f, 100.0f);
  ren_set_camera_projection(PERSPECTIVE);
  ren_set_camera_fov(1.3);
  ren_camera_set_position(0.0, 0.0, 5.0);
  ren_camera_set_rotation(0.0, 0.0, 0.0);
}

int main(int argc, char *argv[]) {

  iom_init();
  ren_init();

  iom_set_event_callback(sdl_callback);
  start_camera();

  // TODO: ERROR HANDLING
  // ren_init();

  s32 p_id = ren_create_program_from_files(SHADERS_SOURCE_DIR "vertex.glsl",
                                           SHADERS_SOURCE_DIR "fragment.glsl");
  s32 e1 = ren_create_entity();
  s32 cube = ren_primitive_create_cube();
  ren_entity_add_component(e1, COMPONENT_OBJECT, cube);
  ren_entity_add_component(e1, COMPONENT_PROGRAM, p_id);

  while (!enable) {
    iom_poll_events();
    ren_draw_frame();
  }

  INFO("shader src dir: %s", SHADERS_SOURCE_DIR "vertex.glsl");
  INFO("SHADER created with id: %d", p_id);

  iom_quit();

  ////ren_create_program(const s8 *vertex_src, const s8 *fragment_src)

  //// ren_draw_frame();
  // s32 r = ren_primitive_create_cube();
  // INFO(r);

  return 0;
};

// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

//
// #include "simple_app.h"

// #if defined(CEF_X11)
// #include <X11/Xlib.h>
// #endif
//
// #include "include/base/cef_logging.h"
// #include "include/cef_command_line.h"
//
// #if defined(CEF_X11)
// namespace {
//
// int XErrorHandlerImpl(Display *display, XErrorEvent *event) {
//  LOG(WARNING) << "X error received: " << "type " << event->type << ", "
//               << "serial " << event->serial << ", " << "error_code "
//               << static_cast<int>(event->error_code) << ", " << "request_code
//               "
//               << static_cast<int>(event->request_code) << ", " << "minor_code
//               "
//               << static_cast<int>(event->minor_code);
//  return 0;
//}
//
// int XIOErrorHandlerImpl(Display *display) { return 0; }
//
//} // namespace
// #endif // defined(CEF_X11)
//
//// Entry point function for all processes.
// NO_STACK_PROTECTOR
// int main(int argc, char *argv[]) {
//   // Provide CEF with command-line arguments.
//   CefMainArgs main_args(argc, argv);
//
//   // SimpleApp implements application-level callbacks. It will create the
//   first
//   // browser instance in OnContextInitialized() after CEF has initialized.
//   CefRefPtr<SimpleApp> app(new SimpleApp);
//
//   // CEF applications have multiple sub-processes (render, GPU, etc) that
//   share
//   // the same executable. This function checks the command-line and, if this
//   is
//   // a sub-process, executes the appropriate logic.
//   int exit_code = CefExecuteProcess(main_args, app, nullptr);
//   if (exit_code >= 0) {
//     // The sub-process has completed so return here.
//     return exit_code;
//   }
//
// #if defined(CEF_X11)
//   // Install xlib error handlers so that the application won't be terminated
//   // on non-fatal errors.
//   XSetErrorHandler(XErrorHandlerImpl);
//   XSetIOErrorHandler(XIOErrorHandlerImpl);
// #endif
//
//   // Parse command-line arguments for use in this method.
//   CefRefPtr<CefCommandLine> command_line =
//   CefCommandLine::CreateCommandLine(); command_line->InitFromArgv(argc,
//   argv);
//
//   // Specify CEF global settings here.
//   CefSettings settings;
//
//// When generating projects with CMake the CEF_USE_SANDBOX value will be
/// defined / automatically. Pass -DUSE_SANDBOX=OFF to the CMake command-line to
/// disable / use of the sandbox.
// #if !defined(CEF_USE_SANDBOX)
//   settings.no_sandbox = true;
// #endif
//
//   // Initialize the CEF browser process. May return false if initialization
//   // fails or if early exit is desired (for example, due to process singleton
//   // relaunch behavior).
//   if (!CefInitialize(main_args, settings, app.get(), nullptr)) {
//     return CefGetExitCode();
//   }
//
//   // Run the CEF message loop. This will block until CefQuitMessageLoop() is
//   // called.
//   CefRunMessageLoop();
//
//   // Shut down CEF.
//   CefShutdown();
//
//   return 0;
// }
