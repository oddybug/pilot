#ifndef RENDER_HANDLER_H
#define RENDER_HANDLER_H

#include "include/cef_app.h"
#include "include/cef_v8.h"
#include <map>
#include <string>
#include <utility>

class MyRenderProcessHandler;

class MyV8Handler : public CefV8Handler {
public:
  explicit MyV8Handler(CefRefPtr<MyRenderProcessHandler> render_handler)
      : render_handler_(render_handler) {}

  bool Execute(const CefString &name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList &arguments, CefRefPtr<CefV8Value> &retval,
               CefString &exception) override;

private:
  CefRefPtr<MyRenderProcessHandler> render_handler_;
  IMPLEMENT_REFCOUNTING(MyV8Handler);
};

class MyRenderProcessHandler : public CefRenderProcessHandler {
  friend class MyV8Handler;

public:
  typedef std::pair<std::string, int> CallbackKey;
  typedef std::pair<CefRefPtr<CefV8Context>, CefRefPtr<CefV8Value>>
      CallbackValue;
  typedef std::map<CallbackKey, CallbackValue> CallbackMap;

  MyRenderProcessHandler() = default;

  void OnContextCreated(CefRefPtr<CefBrowser> browser,
                        CefRefPtr<CefFrame> frame,
                        CefRefPtr<CefV8Context> context) override;

  void OnContextReleased(CefRefPtr<CefBrowser> browser,
                         CefRefPtr<CefFrame> frame,
                         CefRefPtr<CefV8Context> context) override;

  bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefProcessId source_process,
                                CefRefPtr<CefProcessMessage> message) override;

private:
  CallbackMap callback_map_;
  IMPLEMENT_REFCOUNTING(MyRenderProcessHandler);
};

#endif //! RENDER_HANDLER_H
