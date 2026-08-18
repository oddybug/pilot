// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "simple_handler.h"

#include <cstring>
#include <sstream>
#include <string>

#include "data/hashmap.h"
#include "data/hashmap_helpers.h"
#include "include/base/cef_callback.h"
#include "include/cef_app.h"
#include "include/cef_parser.h"
#include "include/internal/cef_types.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"
#include "log.h"
#include "ui_ipc.h"

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
  init_e_map();
}

SimpleHandler::~SimpleHandler() { g_instance = nullptr; }

// static
SimpleHandler *SimpleHandler::GetInstance() { return g_instance; }

CefRefPtr<CefBrowser> SimpleHandler::GetBrowser() const {
  CEF_REQUIRE_UI_THREAD();
  if (!browser_list_.empty()) {
    return browser_list_.front();
  }
  return nullptr;
}

struct test {
  const char *msg[10];
  int id;
  float test;
};

bool SimpleHandler::OnProcessMessageReceived(
    CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
    CefProcessId source_process, CefRefPtr<CefProcessMessage> message) {
  CEF_REQUIRE_UI_THREAD();

  const std::string message_name = message->GetName();

  if (message_name == "binding_test") {
    CefRefPtr<CefProcessMessage> response =
        CefProcessMessage::Create("binding_test");
    CefRefPtr<CefListValue> response_args = response->GetArgumentList();

    response_args->SetString(0, "Hello from Browser Process C++!");
    frame->SendProcessMessage(PID_RENDERER, response);
    return true;
  } else if (message_name == "ipc_dicc_req") {
    CefRefPtr<CefProcessMessage> response =
        CefProcessMessage::Create("ipc_dicc_stream");
    void *args_bs = ui_ipc_stream_get();

    CefRefPtr<CefListValue> response_args = response->GetArgumentList();

    u32 size_n; //(dummi var)
    u32 size = ui_ipc_stream_get_size(&size_n);
    CefRefPtr<CefBinaryValue> msg = CefBinaryValue::Create(args_bs, size);

    response_args->SetBinary(0, msg);
    frame->SendProcessMessage(PID_RENDERER, response);
    return true;

  } else if (message_name == "entry") {

    CefRefPtr<CefListValue> args = message->GetArgumentList();
    CefRefPtr<CefBinaryValue> bs = args->GetBinary(0);
    void *stream = (void *)bs->GetRawData();
    c8 *sc = (c8 *)stream;
    c8 *key;
    strcpy(key, sc);
    sc += strlen(key) + sizeof(c8);
    struct entry_c_T *e = (struct entry_c_T *)gen_map_find(e_map_, key);
    if (e) {

      size_t args_s = ui_ipc_argsv_get(&e->e.out_args);
      size_t msg_s = args_s + strlen(key) + sizeof(c8);
      void *msg[msg_s];
      struct response_T response = {
          .key = key, .args = &e->e.out_args, .msg = msg, .it = msg};
      strcpy((c8 *)response.msg, key);

      ui_ipc_stream_write_string((void **)&response.it, key);

      e->callback((void *)sc, &response);

      if (!e->e.out_args.n_args)
        return true;

      CefRefPtr<CefProcessMessage> cef_response =
          CefProcessMessage::Create("entry_response");
      CefRefPtr<CefListValue> cef_response_args =
          cef_response->GetArgumentList();

      CefRefPtr<CefBinaryValue> bs =
          CefBinaryValue::Create(response.msg, msg_s);
      cef_response_args->SetBinary(0, bs);
      frame->SendProcessMessage(PID_RENDERER, cef_response);
      return true;

    } else {
      ERROR("ipc call for '%s' does not exist", key);
      return false;
    }
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

struct map_T *SimpleHandler::GetEntriesMap() { return e_map_; };

static void pilot_entry_free_entry_(struct entry_T *e);

static void pilot_entry_free_entry_(struct entry_T *e) {
  free(e->in_args.args);
  free(e->out_args.args);
};

static void pilot_entry_free_item_fn_(struct item_T *item);

static void pilot_entry_free_item_fn_(struct item_T *item) {
  struct entry_T *value = (entry_T *)item->value;
  pilot_entry_free_entry_(value);
};

void SimpleHandler::init_e_map() {

  e_map_ = gen_map_create(DICC_SIZE, gen_map_hash_fn_c8p, gen_map_cmp_key_c8p,
                          pilot_entry_free_item_fn_);
};

// TODO: to implement
void SimpleHandler::OnPaint(CefRefPtr<CefBrowser> browser,
                            PaintElementType type, const RectList &dirtyRects,
                            const void *buffer, int width, int height) {

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
