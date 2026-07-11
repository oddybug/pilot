// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "simple_app.h"

#include <iostream>
#include <string>

#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/cef_v8.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_helpers.h"
#include "simple_handler.h"

namespace {

// V8 handler to handle JavaScript function calls in the renderer process
class MyV8Handler : public CefV8Handler {
public:
  MyV8Handler() = default;

  bool Execute(const CefString &name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList &arguments, CefRefPtr<CefV8Value> &retval,
               CefString &exception) override {

    if (name == "sayHello") {
      // Get the argument

      if (arguments.size() == 1 && arguments[0]->IsString()) {
        CefString message = arguments[0]->GetStringValue();

        // Send message to browser process
        CefRefPtr<CefProcessMessage> msg =
            CefProcessMessage::Create("greeting");
        msg->GetArgumentList()->SetString(0, message);

        CefRefPtr<CefV8Context> context = CefV8Context::GetCurrentContext();
        CefRefPtr<CefFrame> frame = context->GetFrame();

        // Results in an asynchronous call to
        // SimpleHandler::OnProcessMessageReceived in the browser process
        frame->SendProcessMessage(PID_BROWSER, msg);

        // Return value to JavaScript
        retval = CefV8Value::CreateString("Message sent to C++!");
        return true;
      }
    }

    return false;
  }

private:
  IMPLEMENT_REFCOUNTING(MyV8Handler);
  DISALLOW_COPY_AND_ASSIGN(MyV8Handler);
};

// When using the Views framework this object provides the delegate
// implementation for the CefWindow that hosts the Views-based browser.
class SimpleWindowDelegate : public CefWindowDelegate {
public:
  SimpleWindowDelegate(CefRefPtr<CefBrowserView> browser_view,
                       cef_runtime_style_t runtime_style,
                       cef_show_state_t initial_show_state)
      : browser_view_(browser_view), runtime_style_(runtime_style),
        initial_show_state_(initial_show_state) {}

  SimpleWindowDelegate(const SimpleWindowDelegate &) = delete;
  SimpleWindowDelegate &operator=(const SimpleWindowDelegate &) = delete;

  void OnWindowCreated(CefRefPtr<CefWindow> window) override {
    // Add the browser view and show the window.
    window->AddChildView(browser_view_);

    if (initial_show_state_ != CEF_SHOW_STATE_HIDDEN) {
      window->Show();
    }
  }

  void OnWindowDestroyed(CefRefPtr<CefWindow> window) override {
    browser_view_ = nullptr;
  }

  bool CanClose(CefRefPtr<CefWindow> window) override {
    // Allow the window to close if the browser says it's OK.
    CefRefPtr<CefBrowser> browser = browser_view_->GetBrowser();
    if (browser) {
      return browser->GetHost()->TryCloseBrowser();
    }
    return true;
  }

  CefSize GetPreferredSize(CefRefPtr<CefView> view) override {
    return CefSize(800, 600);
  }

  cef_show_state_t GetInitialShowState(CefRefPtr<CefWindow> window) override {
    return initial_show_state_;
  }

  cef_runtime_style_t GetWindowRuntimeStyle() override {
    return runtime_style_;
  }

private:
  CefRefPtr<CefBrowserView> browser_view_;
  const cef_runtime_style_t runtime_style_;
  const cef_show_state_t initial_show_state_;

  IMPLEMENT_REFCOUNTING(SimpleWindowDelegate);
};

class SimpleBrowserViewDelegate : public CefBrowserViewDelegate {
public:
  explicit SimpleBrowserViewDelegate(cef_runtime_style_t runtime_style)
      : runtime_style_(runtime_style) {}

  SimpleBrowserViewDelegate(const SimpleBrowserViewDelegate &) = delete;
  SimpleBrowserViewDelegate &
  operator=(const SimpleBrowserViewDelegate &) = delete;

  bool OnPopupBrowserViewCreated(CefRefPtr<CefBrowserView> browser_view,
                                 CefRefPtr<CefBrowserView> popup_browser_view,
                                 bool is_devtools) override {
    // Create a new top-level Window for the popup. It will show itself after
    // creation.
    CefWindow::CreateTopLevelWindow(new SimpleWindowDelegate(
        popup_browser_view, runtime_style_, CEF_SHOW_STATE_NORMAL));

    // We created the Window.
    return true;
  }

  cef_runtime_style_t GetBrowserRuntimeStyle() override {
    return runtime_style_;
  }

private:
  const cef_runtime_style_t runtime_style_;

  IMPLEMENT_REFCOUNTING(SimpleBrowserViewDelegate);
};

} // namespace

SimpleApp::SimpleApp() = default;

extern void ui_register_handler(CefRefPtr<SimpleHandler> handler);

void SimpleApp::OnContextInitialized() {
  CEF_REQUIRE_UI_THREAD();

  CefRefPtr<CefCommandLine> command_line =
      CefCommandLine::GetGlobalCommandLine();

  // Check if Alloy style will be used.
  cef_runtime_style_t runtime_style = CEF_RUNTIME_STYLE_ALLOY;

  // SimpleHandler implements browser-level callbacks.
  CefRefPtr<SimpleHandler> handler(new SimpleHandler(CEF_RUNTIME_STYLE_ALLOY));

  ui_register_handler(handler);

  // Specify CEF browser settings here.
  CefBrowserSettings browser_settings;

  std::string url;

  // Check if a "--url=" value was provided via the command-line. If so, use
  // that instead of the default URL.
  url = command_line->GetSwitchValue("url");
  if (url.empty()) {
    url = "file:///home/oddy/repos/pilot/ui/index.html"; // TODO: THIS DIR IS
                                                         // HARDCODED !URGENT
  }

  // Information used when creating the native window.
  CefWindowInfo window_info;

#if defined(OS_WIN)
  // On Windows we need to specify certain flags that will be passed to
  // CreateWindowEx().
  window_info.SetAsPopup(nullptr, "cefsimple");
#endif

  // Alloy style will create a basic native window. Chrome style will create a
  // fully styled Chrome UI window.
  window_info.runtime_style = runtime_style;

  // Tell window we want OSR mode
  // TODO: Some features that requires parent window as specified in the api
  // of this call will not work.
  window_info.SetAsWindowless(0);

  // Create the first browser window.
  CefBrowserHost::CreateBrowser(window_info, handler, url, browser_settings,
                                nullptr, nullptr);
  // CefBrowserHost::CloseBrowser();
}

// Called when the JavaScript context is created in the renderer process
void SimpleApp::OnContextCreated(CefRefPtr<CefBrowser> browser,
                                 CefRefPtr<CefFrame> frame,
                                 CefRefPtr<CefV8Context> context) {
  // Debug assertion - crashes if called on wrong thread
  // V8 APIs must only be called from the renderer thread
  CEF_REQUIRE_RENDERER_THREAD();

  // Register JavaScript function 'sayHello' on the global object
  // (window.sayHello)
  CefRefPtr<CefV8Value> object = context->GetGlobal();
  CefRefPtr<CefV8Handler> handler = new MyV8Handler();

  CefRefPtr<CefV8Value> func = CefV8Value::CreateFunction("sayHello", handler);
  object->SetValue("sayHello", func, V8_PROPERTY_ATTRIBUTE_NONE);
}

CefRefPtr<CefClient> SimpleApp::GetDefaultClient() {
  // Called when a new browser window is created via Chrome style UI.
  return SimpleHandler::GetInstance();
}
