// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_
#define CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_

#include <list>

#include "data/list.h"
#include "include/cef_client.h"
#include "include/cef_render_handler.h"
#include "ui_msg_common.h"

#include "types.h"

#include "data/hashmap.h"

class SimpleHandler : public CefClient,
                      public CefDisplayHandler,
                      public CefLifeSpanHandler,
                      public CefLoadHandler,
                      public CefRenderHandler {
public:
  explicit SimpleHandler(bool is_alloy_style);
  ~SimpleHandler() override;

  // Provide access to the single global instance of this object.
  static SimpleHandler *GetInstance();

  // CefClient methods:
  CefRefPtr<CefDisplayHandler> GetDisplayHandler() override { return this; }
  CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }
  CefRefPtr<CefLoadHandler> GetLoadHandler() override { return this; }
  CefRefPtr<CefRenderHandler> GetRenderHandler() override { return this; }

  CefRefPtr<CefBrowser> GetBrowser() const;

  // Handle incoming messages
  bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefProcessId source_process,
                                CefRefPtr<CefProcessMessage> message) override;

  // CefDisplayHandler methods:
  void OnTitleChange(CefRefPtr<CefBrowser> browser,
                     const CefString &title) override;

  // CefLifeSpanHandler methods:
  void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
  bool DoClose(CefRefPtr<CefBrowser> browser) override;
  void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

  void CloseAllBrowsers();

  // CefLoadHandler methods:
  void OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                   ErrorCode errorCode, const CefString &errorText,
                   const CefString &failedUrl) override;

  void ShowMainWindow();

  bool AreAllBrowsersClosed();

  using TextureCallbackFn = void (*)(u8 *, u32, u32);

  void SetTextureCallback(TextureCallbackFn clbk);

  void ResizeBrowsers(u32 width, u32 height);

  map_T GetPullMsgMap();

  map_T GetPushMsgMap();

  void SendPushMsg(list_T list, msg_T msg);

private:
  void init_e_map();
  map_T pull_msg_m_;
  map_T push_msg_m_;
  bool is_closed_ = false;

  std::map<std::string, CefRefPtr<CefFrame>> frames;
  u32 window_w = 0;
  u32 window_h = 0;

  // Platform-specific implementation.
  void PlatformTitleChange(CefRefPtr<CefBrowser> browser,
                           const CefString &title);
  void PlatformShowWindow(CefRefPtr<CefBrowser> browser);

  // True if this client is Alloy style, otherwise Chrome style.
  const bool is_alloy_style_;

  // List of existing browser windows. Only accessed on the CEF UI thread.
  typedef std::list<CefRefPtr<CefBrowser>> BrowserList;
  BrowserList browser_list_;

  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(SimpleHandler);

public:
  // SUPORT FOR OSR -- CefRenderHandler
  // TODO: SOME METHODS ARE NOT IMPLEMENTED AND ARE LEFT EMPTY

  CefRefPtr<CefAccessibilityHandler> GetAccessibilityHandler() override;

  bool GetRootScreenRect(CefRefPtr<CefBrowser> browser, CefRect &rect) override;

  void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect) override;

  bool GetScreenPoint(CefRefPtr<CefBrowser> browser, int viewX, int viewY,
                      int &screenX, int &screenY) override;

  bool GetScreenInfo(CefRefPtr<CefBrowser> browser,
                     CefScreenInfo &screen_info) override;

  void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) override;

  void OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect &rect) override;

  void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type,
               const RectList &dirtyRects, const void *buffer, int width,
               int height) override;

  void OnAcceleratedPaint(CefRefPtr<CefBrowser> browser, PaintElementType type,
                          const RectList &dirtyRects,
                          const CefAcceleratedPaintInfo &info) override;

  void GetTouchHandleSize(CefRefPtr<CefBrowser> browser,
                          cef_horizontal_alignment_t orientation,
                          CefSize &size) override;

  void OnTouchHandleStateChanged(CefRefPtr<CefBrowser> browser,
                                 const CefTouchHandleState &state) override;
  bool StartDragging(CefRefPtr<CefBrowser> browser,
                     CefRefPtr<CefDragData> drag_data,
                     DragOperationsMask allowed_ops, int x, int y) override;

  void UpdateDragCursor(CefRefPtr<CefBrowser> browser,
                        DragOperation operation) override;

  void OnScrollOffsetChanged(CefRefPtr<CefBrowser> browser, double x,
                             double y) override;
  void OnImeCompositionRangeChanged(CefRefPtr<CefBrowser> browser,
                                    const CefRange &selected_range,
                                    const RectList &character_bounds) override;
  void OnTextSelectionChanged(CefRefPtr<CefBrowser> browser,
                              const CefString &selected_text,
                              const CefRange &selected_range) override;
  void OnVirtualKeyboardRequested(CefRefPtr<CefBrowser> browser,
                                  TextInputMode input_mode) override;

  //
  // END OF CefRenderHandler INTERFACE
  //

private:
  TextureCallbackFn text_callback_ = nullptr;
};

#endif // CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_
