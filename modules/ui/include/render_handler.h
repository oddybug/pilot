#ifndef RENDER_HANDLER_H
#define RENDER_HANDLER_H

#include "data/hashmap.h"
#include "include/cef_app.h"
#include "include/cef_v8.h"
#include "include/cef_values.h"
#include "ui_ipc.h"
#include <cstddef>
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

  CefRefPtr<CefListValue> ConvertV8ListToCefList(const CefV8ValueList &v8List);
  CefRefPtr<CefValue> CefV8ValueToCefValue(CefRefPtr<CefV8Value> v8Value);
};

class MyRenderProcessHandler : public CefRenderProcessHandler {
  friend class MyV8Handler;

public:
  static MyRenderProcessHandler *GetInstance();

  typedef std::pair<std::string, int> CallbackKey;
  typedef std::pair<CefRefPtr<CefV8Context>, CefRefPtr<CefV8Value>>
      CallbackValue;
  typedef std::map<CallbackKey, CallbackValue> CallbackMap;

  MyRenderProcessHandler();
  ~MyRenderProcessHandler();

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
  struct map_T *GetPullMsgMap();

private:
  struct map_T *msg_pull_m_;
  void init_msg_pull_m_();

  bool CheckType(enum ARG_TYPE c_type, CefValueType js_type);

  CefRefPtr<CefProcessMessage> CreateMessage(const c8 *name,
                                             struct pull_msg_e_render *e,
                                             CefRefPtr<CefListValue> args);

  void CreateMessageBs(msg_T msg, const c8 *name, struct args *in,
                       CefRefPtr<CefListValue> &args);

  void CopyValueToStream(CefRefPtr<CefValue> &value, msg_T msg);

  void PushArgument(CefV8ValueList &arguments, void *value, enum ARG_TYPE type);

  CallbackMap callback_map_;
  IMPLEMENT_REFCOUNTING(MyRenderProcessHandler);
};

#endif //! RENDER_HANDLER_H
