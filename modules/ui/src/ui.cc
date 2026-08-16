// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "ui.h"

#include "include/cef_app.h"
#include "include/cef_command_line.h"
#include "include/cef_task.h"
#include "include/internal/cef_ptr.h"
#include "include/internal/cef_types.h"
#include "include/internal/cef_types_wrappers.h"
#include "include/wrapper/cef_helpers.h"
#include "log.h"
#include "simple_app.h"
#include "simple_handler.h"
#include "ui_ipc.h"
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
    return CefGetExitCode();
  }

  return 0;
};

// TODO: check if cef is started
void ui_message_loop() { CefDoMessageLoopWork(); };

void ui_send_mouse_keydown(c16 key) {

  CefRefPtr<SimpleHandler> handler = SimpleHandler::GetInstance();
  CefRefPtr<CefBrowser> browser = handler->GetBrowser();

  if (!browser)
    return;

  CefKeyEvent k_e;
  k_e.type = KEYEVENT_RAWKEYDOWN;
  k_e.windows_key_code = key;
  k_e.native_key_code = key;

  browser->GetHost()->SendKeyEvent(k_e);

  k_e.type = KEYEVENT_CHAR;
  k_e.character = key;
  k_e.unmodified_character = key;

  browser->GetHost()->SendKeyEvent(k_e);
}

extern void ui_send_mouse_keyup(c16 key) {
  CefRefPtr<SimpleHandler> handler = SimpleHandler::GetInstance();
  CefRefPtr<CefBrowser> browser = handler->GetBrowser();

  if (!browser)
    return;

  CefKeyEvent k_e;
  k_e.type = KEYEVENT_KEYUP;
  k_e.windows_key_code = key;
  k_e.native_key_code = key;

  browser->GetHost()->SendKeyEvent(k_e);
};

void ui_send_mouse_event_click(enum MOUSE_BTN mb, struct point_T m_p) {

  CefRefPtr<SimpleHandler> handler = SimpleHandler::GetInstance();
  CefRefPtr<CefBrowser> browser = handler->GetBrowser();

  if (!browser)
    return;

  CefMouseEvent m_e;
  m_e.x = m_p.x;
  m_e.y = m_p.y;

  browser->GetHost()->SendMouseClickEvent(m_e, (cef_mouse_button_type_t)mb,
                                          false, 1);

  browser->GetHost()->SendMouseClickEvent(m_e, (cef_mouse_button_type_t)mb,
                                          true, 1);
};

void ui_send_mouse_event_motion(struct point_T m_p) {

  CefMouseEvent m_e;

  CefRefPtr<SimpleHandler> handler = SimpleHandler::GetInstance();
  CefRefPtr<CefBrowser> browser = handler->GetBrowser();
  // auto host = handler.bro

  if (!browser)
    return;

  m_e.x = m_p.x;
  m_e.y = m_p.y;
  browser->GetHost()->SendMouseMoveEvent(m_e, false);
};

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

struct map_T *ui_ipc_get_browser_map() {

  CefRefPtr<SimpleHandler> handler = SimpleHandler::GetInstance();
  return handler->GetEntriesMap();
};

void ui_close() {
  ui_ipc_free();
  CefShutdown();
};
