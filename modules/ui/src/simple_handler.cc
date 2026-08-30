// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "simple_handler.h"

#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>

#include "data/hashmap.h"
#include "data/hashmap_helpers.h"
#include "data/list.h"
#include "include/base/cef_callback.h"
#include "include/cef_app.h"
#include "include/cef_parser.h"
#include "include/internal/cef_types.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"
#include "log.h"
#include "ui_msg_browser.h"
#include "ui_msg_common.h"

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

// extern msg_T ui_msg_create_(const c8 *name, struct args *args);

bool SimpleHandler::OnProcessMessageReceived(
    CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
    CefProcessId source_process, CefRefPtr<CefProcessMessage> message) {
  CEF_REQUIRE_UI_THREAD();

  const std::string message_name = message->GetName();

  if (message_name == "ipc_dicc_req") {

    struct map_it_T it;
    gen_map_it_set_begin(pull_msg_m_, &it);
    msg_map_T msg = ui_msg_pull_bm_e(&it);
    while (msg) {
      CefRefPtr<CefProcessMessage> response =
          CefProcessMessage::Create("ipc_dicc_stream");
      CefRefPtr<CefListValue> response_args = response->GetArgumentList();
      CefRefPtr<CefBinaryValue> cef_msg =
          CefBinaryValue::Create(ui_msg_map_bs(msg), ui_msg_map_size(msg));
      response_args->SetBinary(0, cef_msg);
      frame->SendProcessMessage(PID_RENDERER, response);
      ui_msg_map_free(msg);
      msg = ui_msg_pull_bm_e(&it);
    }
    return true;
  } else if (message_name == "entry") {

    CefRefPtr<CefListValue> args = message->GetArgumentList();
    CefRefPtr<CefBinaryValue> bs = args->GetBinary(0);
    c8 sc[bs->GetSize()];
    void *stream = (void *)sc;
    bs->GetData(stream, bs->GetSize(), 0);

    size_t key_s = strlen(sc) + sizeof(c8);
    c8 *key = (c8 *)malloc(key_s * sizeof(c8));
    strcpy(key, sc);

    struct pull_msg_bme *e =
        (struct pull_msg_bme *)gen_map_find(pull_msg_m_, key);
    if (!e) {
      WARN("ipc call for '%s' does not exist", key);
      return false;
    }

    msg_T msg = ui_msg_get_fs(stream, &e->in);
    if (!msg) {
      WARN("msg couldnt get created");
      return false;
    }

    msg_T response = ui_msg_create_(key, &e->out);
    e->callback(msg, response);

    if (!e->out.n_args) {
      free(key);
      return true;
    }

    CefRefPtr<CefProcessMessage> cef_response =
        CefProcessMessage::Create("entry_response");
    CefRefPtr<CefListValue> cef_response_args = cef_response->GetArgumentList();

    CefRefPtr<CefBinaryValue> bs_res = CefBinaryValue::Create(
        ui_msg_bitstream(response), ui_msg_size(response));
    cef_response_args->SetBinary(0, bs_res);
    frame->SendProcessMessage(PID_RENDERER, cef_response);

    free(msg);
    free(response);
    free(key);
    return true;
  } else if (message_name == "push_msg_req") {

    CefRefPtr<CefProcessMessage> response =
        CefProcessMessage::Create("push_msg_req");

    CefRefPtr<CefBinaryValue> bs = message->GetArgumentList()->GetBinary(0);
    bs->GetSize();
    c8 sc[bs->GetSize()];
    void *stream = (void *)sc;
    bs->GetData(stream, bs->GetSize(), 0);

    c8 msg_name[strlen(sc) + 1];
    strcpy(msg_name, sc);

    struct push_msg_bme *pmb =
        (struct push_msg_bme *)gen_map_find(push_msg_m_, msg_name);

    msg_T msg_res;
    s32 msg_s;
    if (!pmb) {
      WARN("No push msg entry with name: %d", msg_name);

      enum ARG_TYPE a[] = {U32};
      struct args args = {.args = a, .n_args = 1};

      msg_res = ui_msg_create_(msg_name, &args);
      s32 num = -1;
      ui_msg_push_s32_r(msg_res, num);

      msg_s = ui_msg_size(msg_res);
    } else {
      // create entry
      if (!pmb->render) {
        pmb->render = gen_list_new();
      }
      std::string id_s = frame->GetIdentifier();
      const c8 *id_c = id_s.c_str();
      c8 *id = (c8 *)malloc(id_s.size() + 1);
      if (!id) {
        ERROR("failed to allocate memory. errno: %d", errno);
        return false;
      }
      strcpy(id, id_c);
      gen_list_push_front(pmb->render, id);

      u32 msg_nargs = pmb->out.n_args + 1;
      enum ARG_TYPE a[msg_nargs];
      a[0] = S32;
      int i;
      for (i = 1; i < msg_nargs; i++) {
        a[i] = ARG_TYPE;
      }
      struct args args = {.args = a, .n_args = msg_nargs};
      msg_res = ui_msg_create_(msg_name, &args);

      ui_msg_push_s32_r(msg_res, pmb->out.n_args);
      for (i = 0; i < pmb->out.n_args; i++)
        ui_msg_push_s32_r(msg_res, (s32)pmb->out.args[i]);

      msg_s = ui_msg_size(msg_res);
    }

    CefRefPtr<CefListValue> args = response->GetArgumentList();
    CefRefPtr<CefBinaryValue> res_bs =
        CefBinaryValue::Create(ui_msg_bitstream(msg_res), msg_s);
    args->SetBinary(0, res_bs);
    frame->SendProcessMessage(PID_RENDERER, response);
    std::string fid = frame->GetIdentifier();
    frames[fid] = frame;

    free(msg_res);
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

map_T SimpleHandler::GetPullMsgMap() { return pull_msg_m_; };

map_T SimpleHandler::GetPushMsgMap() { return push_msg_m_; };

void SimpleHandler::SendPushMsg(list_T list, msg_T msg) {
  assert(list && msg);
  // gen_list_find(list , const void *key)
  struct node *n = gen_list_first(list);
  if (!n)
    return;

  while (n) {
    c8 name[strlen((c8 *)n->value) + 1];
    std::string id = (c8 *)n->value;
    auto it = frames.find(id);

    if (it == frames.end()) {
      ERROR("Frame not found %s", name);
      break;
    }

    CefRefPtr<CefFrame> frame = it->second;
    // TODO: I DONT KNOW IF BS CAN BE OUT OF THE LOOP TO TRY
    CefRefPtr<CefBinaryValue> bs =
        CefBinaryValue::Create(ui_msg_bitstream(msg), ui_msg_size(msg));
    CefRefPtr<CefProcessMessage> cef_msg =
        CefProcessMessage::Create("push_msg");
    cef_msg->GetArgumentList()->SetBinary(0, bs);

    frame->SendProcessMessage(PID_RENDERER, cef_msg);
    frames[id];

    n = n->next;
  }
};

void SimpleHandler::init_e_map() {
  pull_msg_m_ = gen_map_create(DICC_SIZE, gen_map_hash_fn_c8p,
                               gen_map_cmp_key_c8p, ui_msg_pullem_free_clbk_);

  push_msg_m_ = gen_map_create(DICC_SIZE, gen_map_hash_fn_c8p,
                               gen_map_cmp_key_c8p, ui_msg_pushem_free_clbk_);
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
