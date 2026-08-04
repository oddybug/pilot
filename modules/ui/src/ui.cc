// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "ui.h"

#include "include/cef_app.h"
#include "include/cef_command_line.h"
#include "include/internal/cef_ptr.h"
#include "include/wrapper/cef_helpers.h"
#include "log.h"
#include "simple_app.h"
#include "simple_handler.h"
#include <cstdlib>

s32 ui_start(int argc, char *argv[]) {
  // Create a copy of |argv| on Linux because Chromium mangles the value
  // internally (see issue #620).
  CefScopedArgArray scoped_arg_array(argc, argv);
  char **argv_copy = scoped_arg_array.array();

  CefMainArgs main_args(argc, argv);

  // SimpleApp implements application-level callbacks. It will create the first
  // browser instance in OnContextInitialized() after CEF has initialized.
  CefRefPtr<SimpleApp> app = CefRefPtr<SimpleApp>(new SimpleApp);
  // CEF applications have multiple sub-processes (render, GPU, etc) that share
  // the same executable. This function checks the command-line and, if this is
  // a sub-process, executes the appropriate logic.
  int exit_code = CefExecuteProcess(main_args, app, nullptr);
  if (exit_code >= 0) {
    // The sub-process has completed so return here.
    exit(exit_code);
  }

  // Parse command-line arguments for use in this method.
  CefRefPtr<CefCommandLine> command_line = CefCommandLine::CreateCommandLine();
  command_line->InitFromArgv(argc, argv);

  // Specify CEF global settings here.
  CefSettings settings;
  settings.windowless_rendering_enabled = true;

  // TODO: not crossplatform
  CefString(&settings.root_cache_path).FromString("/tmp/pilot_cef_cache");

  // When generating projects with CMake the CEF_USE_SANDBOX value will be
  // defined / automatically. Pass -DUSE_SANDBOX=OFF to the CMake command-line
  // to disable / use of the sandbox.
#if !defined(CEF_USE_SANDBOX)
  settings.no_sandbox = true;
#endif

  // Initialize the CEF browser process. May return false if initialization
  // fails or if early exit is desired (for example, due to process singleton
  // relaunch behavior).
  if (!CefInitialize(main_args, settings, app.get(), nullptr)) {
    return -1; // TODO: CefGetExitCode();
  }

  return 0;
};

// TODO: check if cef is started
void ui_message_loop() { CefDoMessageLoopWork(); };

bool ui_can_close() {
  CefRefPtr<SimpleHandler> handler = SimpleHandler::GetInstance();
  return handler->AreAllBrowsersClosed();
};

void ui_close_browsers() {
  CefRefPtr<SimpleHandler> handler = SimpleHandler::GetInstance();
  handler->CloseAllBrowsers();
}

void ui_set_ui_texture_callback(void (*clbk)(u8 *buffer, u32 width,
                                             u32 height)) {
  SimpleHandler::GetInstance()->SetTextureCallback(clbk);
};

void ui_resize_window(u32 width, u32 height) {
  CefRefPtr<SimpleHandler> handler = SimpleHandler::GetInstance();
  handler->ResizeBrowsers(width, height);
};

void ui_close() { CefShutdown(); };
