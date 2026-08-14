// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFSIMPLE_SIMPLE_APP_H_
#define CEF_TESTS_CEFSIMPLE_SIMPLE_APP_H_

#include "include/cef_app.h"
#include "include/cef_render_process_handler.h"

#include "render_handler.h"

// Implement application-level callbacks for the browser process.
class SimpleApp : public CefApp, public CefBrowserProcessHandler {
public:
  SimpleApp();
  // CefApp methods:
  CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override {
    return this;
  }

  CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override {
    return render_process_handler_;
  }

  // CefBrowserProcessHandler methods:
  void OnContextInitialized() override;

  CefRefPtr<CefClient> GetDefaultClient() override;

  virtual void OnBeforeCommandLineProcessing(
      const CefString &process_type,
      CefRefPtr<CefCommandLine> command_line) override;

private:

  void SendIpcDicc(CefRefPtr<CefBrowser> browser);

  CefRefPtr<MyRenderProcessHandler> render_process_handler_;

  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(SimpleApp);
};

#endif // CEF_TESTS_CEFSIMPLE_SIMPLE_APP_H_
