// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "simple_handler.h"

#include <sstream>
#include <string>

#include "include/base/cef_callback.h"
#include "include/cef_app.h"
#include "include/cef_parser.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"
#include "log.h"

namespace {

SimpleHandler *g_instance = nullptr;

// Returns a data: URI with the specified contents.
std::string GetDataURI(const std::string &data, const std::string &mime_type) {
  return "data:" + mime_type + ";base64," +
         CefURIEncode(CefBase64Encode(data.data(), data.size()), false)
             .ToString();
}

} // namespace

SimpleHandler::SimpleHandler(bool is_alloy_style)
    : is_alloy_style_(is_alloy_style) {
  DCHECK(!g_instance);
  g_instance = this;
}

SimpleHandler::~SimpleHandler() { g_instance = nullptr; }

// static
SimpleHandler *SimpleHandler::GetInstance() { return g_instance; }

bool SimpleHandler::OnProcessMessageReceived(
    CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
    CefProcessId source_process, CefRefPtr<CefProcessMessage> message) {
  CEF_REQUIRE_UI_THREAD();

  // Handle message from MyHandler::Execute() in the renderer process
  if (message->GetName() == "greeting") {
    CefString msg = message->GetArgumentList()->GetString(0);

    // Show the message in a JavaScript alert
    std::stringstream ss;
    ss << "alert('C++ received: " << msg.ToString() << "');";
    frame->ExecuteJavaScript(ss.str(), frame->GetURL(), 0);

    return true;
  }

  return false;
}

void SimpleHandler::OnTitleChange(CefRefPtr<CefBrowser> browser,
                                  const CefString &title) {
  CEF_REQUIRE_UI_THREAD();

  if (auto browser_view = CefBrowserView::GetForBrowser(browser)) {
    // Set the title of the window using the Views framework.
    CefRefPtr<CefWindow> window = browser_view->GetWindow();
    if (window) {
      window->SetTitle(title);
    }
  } else if (is_alloy_style_) {
    // Set the title of the window using platform APIs.
    PlatformTitleChange(browser, title);
  }
}

void SimpleHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  // Sanity-check the configured runtime style.
  CHECK_EQ(is_alloy_style_ ? CEF_RUNTIME_STYLE_ALLOY : CEF_RUNTIME_STYLE_CHROME,
           browser->GetHost()->GetRuntimeStyle());

  // Add to the list of existing browsers.
  browser_list_.push_back(browser);
}

bool SimpleHandler::DoClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();
  return true;
}

void SimpleHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  // Remove from the list of existing browsers.
  BrowserList::iterator bit = browser_list_.begin();
  for (; bit != browser_list_.end(); ++bit) {
    if ((*bit)->IsSame(browser)) {
      browser_list_.erase(bit);
      break;
    }
  }

  if (browser_list_.empty()) {
    // All browser windows have closed. Quit the application message loop.
    // CefQuitMessageLoop();
  }
}

void SimpleHandler::CloseAllBrowsers() {
  CEF_REQUIRE_UI_THREAD();

  INFO("already clear");
  if (browser_list_.empty())
    return;

  INFO("closing brws");
  for (auto &browser : browser_list_) {
    browser->GetHost()->CloseBrowser(true);
  }
};

void SimpleHandler::OnLoadError(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame, ErrorCode errorCode,
                                const CefString &errorText,
                                const CefString &failedUrl) {
  CEF_REQUIRE_UI_THREAD();

  // Allow Chrome to show the error page.
  if (!is_alloy_style_) {
    return;
  }

  // Don't display an error for downloaded files.
  if (errorCode == ERR_ABORTED) {
    return;
  }

  // Display a load error message using a data: URI.
  std::stringstream ss;
  ss << "<html><body bgcolor=\"white\">"
        "<h2>Failed to load URL "
     << std::string(failedUrl) << " with error " << std::string(errorText)
     << " (" << errorCode << ").</h2></body></html>";

  frame->LoadURL(GetDataURI(ss.str(), "text/html"));
}

void SimpleHandler::ShowMainWindow() {
  if (!CefCurrentlyOn(TID_UI)) {
    // Execute on the UI thread.
    CefPostTask(TID_UI, base::BindOnce(&SimpleHandler::ShowMainWindow, this));
    return;
  }

  if (browser_list_.empty()) {
    return;
  }

  auto main_browser = browser_list_.front();

  if (auto browser_view = CefBrowserView::GetForBrowser(main_browser)) {
    // Show the window using the Views framework.
    if (auto window = browser_view->GetWindow()) {
      window->Show();
    }
  } else if (is_alloy_style_) {
    PlatformShowWindow(main_browser);
  }
}

#if !defined(OS_MAC)
void SimpleHandler::PlatformShowWindow(CefRefPtr<CefBrowser> browser) {
  NOTIMPLEMENTED();
}
#endif

bool SimpleHandler::AreAllBrowsersClosed() { return is_closed_; };

// CefRenderHandler IMPLEMENTATIONS

// TODO: to implement
CefRefPtr<CefAccessibilityHandler> SimpleHandler::GetAccessibilityHandler() {
  return nullptr;
}

// TODO: to implement
bool SimpleHandler::GetRootScreenRect(CefRefPtr<CefBrowser> browser,
                                      CefRect &rect) {
  return false;
}

// TODO: to implement
void SimpleHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect) {
  // TODO: SET AS DESIRED HARDCODED !URGENT
  assert(window_h != 0);
  assert(window_w != 0);

  rect.Set(0, 0, window_w, window_h);
}

// TODO: to implement
bool SimpleHandler::GetScreenPoint(CefRefPtr<CefBrowser> browser, int viewX,
                                   int viewY, int &screenX, int &screenY) {
  return false;
}

// TODO: to implement
bool SimpleHandler::GetScreenInfo(CefRefPtr<CefBrowser> browser,
                                  CefScreenInfo &screen_info) {
  return false;
}

// TODO: to implement
void SimpleHandler::OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) {}

// TODO: to implement
void SimpleHandler::OnPopupSize(CefRefPtr<CefBrowser> browser,
                                const CefRect &rect) {}

void SimpleHandler::SetTextureCallback(TextureCallbackFn clbk) {
  text_callback_ = clbk;
};

void SimpleHandler::ResizeBrowsers(u32 width, u32 height) {
  window_w = width;
  window_h = height;

  for (auto browser : browser_list_) {
    if (browser && browser->GetHost()) {
      browser->GetHost()->WasResized();
    }
  }
};

// TODO: to implement
void SimpleHandler::OnPaint(CefRefPtr<CefBrowser> browser,
                            PaintElementType type, const RectList &dirtyRects,
                            const void *buffer, int width, int height) {

  INFO("COPY PIXELS ENABLED");
  text_callback_((u8 *)buffer, width, height);
}

// TODO: to implement
void SimpleHandler::OnAcceleratedPaint(CefRefPtr<CefBrowser> browser,
                                       PaintElementType type,
                                       const RectList &dirtyRects,
                                       const CefAcceleratedPaintInfo &info) {
  // Shared texture
  INFO("SHARED TEXTURE ENABLED");
}

// TODO: to implement
void SimpleHandler::GetTouchHandleSize(CefRefPtr<CefBrowser> browser,
                                       cef_horizontal_alignment_t orientation,
                                       CefSize &size) {}

// TODO: to implement
void SimpleHandler::OnTouchHandleStateChanged(
    CefRefPtr<CefBrowser> browser, const CefTouchHandleState &state) {}

// TODO: to implement
bool SimpleHandler::StartDragging(CefRefPtr<CefBrowser> browser,
                                  CefRefPtr<CefDragData> drag_data,
                                  DragOperationsMask allowed_ops, int x,
                                  int y) {
  return false;
}

// TODO: to implement
void SimpleHandler::UpdateDragCursor(CefRefPtr<CefBrowser> browser,
                                     DragOperation operation) {}

// TODO: to implement
void SimpleHandler::OnScrollOffsetChanged(CefRefPtr<CefBrowser> browser,
                                          double x, double y) {}

// TODO: to implement
void SimpleHandler::OnImeCompositionRangeChanged(
    CefRefPtr<CefBrowser> browser, const CefRange &selected_range,
    const RectList &character_bounds) {}

// TODO: to implement
void SimpleHandler::OnTextSelectionChanged(CefRefPtr<CefBrowser> browser,
                                           const CefString &selected_text,
                                           const CefRange &selected_range) {}

// TODO: to implement
void SimpleHandler::OnVirtualKeyboardRequested(CefRefPtr<CefBrowser> browser,
                                               TextInputMode input_mode) {}
