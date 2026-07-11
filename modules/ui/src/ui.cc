// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "ui.h"

#include "include/cef_app.h"
#include "include/cef_command_line.h"
#include "include/internal/cef_ptr.h"
#include "simple_app.h"
#include "simple_handler.h"
#include <cstdlib>

enum AppState { STATE_RUNNING, STATE_CLOSING_BROWSERS, STATE_READY_TO_EXIT };

AppState g_app_state = STATE_RUNNING;

static CefRefPtr<SimpleApp> app;
static CefRefPtr<SimpleHandler> g_handler = nullptr;

s32 ui_start(int argc, char *argv[]) {

  // Provide CEF with command-line arguments.
  CefMainArgs main_args(argc, argv);

  // SimpleApp implements application-level callbacks. It will create the first
  // browser instance in OnContextInitialized() after CEF has initialized.
  app = CefRefPtr<SimpleApp>(new SimpleApp);
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
    return -1; // CefGetExitCode();
  }

  // Run the CEF message loop. This will block until CefQuitMessageLoop() is
  // called.
  // CefRunMessageLoop();

  // CefDoMessageLoopWork();

  return 0;
};

// TODO: check if cef is started
void ui_message_loop() { CefDoMessageLoopWork(); };

void ui_register_handler(CefRefPtr<SimpleHandler> handler) {
  g_handler = handler;
}

void ui_close() {
  if (g_app_state != STATE_RUNNING)
    return;
  if (g_handler.get()) {
    g_app_state = STATE_CLOSING_BROWSERS;
    g_handler.get()->CloseAllBrowsers(1);
  } else {
    g_app_state = STATE_READY_TO_EXIT;
  }
}
