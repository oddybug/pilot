#include "render_handler.h"
#include "log.h"

class MyRenderProcessHandler;

bool MyV8Handler::Execute(const CefString &name, CefRefPtr<CefV8Value> object,
                          const CefV8ValueList &arguments,
                          CefRefPtr<CefV8Value> &retval, CefString &exception) {

  // Here i neeed a reference to the hashmap metioned before so i know
  // beforehand the types.
  // But im in the renderer process and i have no acces to it. Need someway to
  // acced earlier than CefInitialize(). I think we can do it with ease.
  //
  //
  // So basicaly the entry_T needs: (input args - c function callback - output
  // args(res))
  //
  // A wrapper needs to be maded for CEF because NO WAY i let somebody deal with
  // CEF API

  if (name == "setMessageCallback") {
  // variadic n of elements
    if (arguments.size() == 2 && arguments[0]->IsString() &&
        arguments[1]->IsFunction()) {
      std::string message_name = arguments[0]->GetStringValue();
      CefRefPtr<CefV8Context> context = CefV8Context::GetCurrentContext();

      if (context.get() && context->IsValid()) {
        int browser_id = context->GetBrowser()->GetIdentifier();
        MyRenderProcessHandler::CallbackKey key =
            std::make_pair(message_name, browser_id);

        if (render_handler_.get()) {
          context->Enter();
          render_handler_->callback_map_[key] =
              std::make_pair(context, arguments[1]);
          context->Exit();
        }

        CefRefPtr<CefProcessMessage> msg =
            CefProcessMessage::Create(message_name);
        context->GetFrame()->SendProcessMessage(PID_BROWSER, msg);
        return true;
      }
    }
  }
  return false;
}

struct test {
  const char *msg[10];
  int id;
  float test;
};

void MyRenderProcessHandler::OnContextCreated(CefRefPtr<CefBrowser> browser,
                                              CefRefPtr<CefFrame> frame,
                                              CefRefPtr<CefV8Context> context) {

  if (!context.get() || !context->IsValid())
    return;

  context->Enter();

  CefRefPtr<CefV8Value> global = context->GetGlobal();

  CefRefPtr<CefV8Value> app = global->GetValue("app");
  if (!app.get() || !app->IsObject()) {
    app = CefV8Value::CreateObject(nullptr, nullptr);
    global->SetValue("app", app, V8_PROPERTY_ATTRIBUTE_NONE);
  }

  CefRefPtr<CefV8Handler> v8_handler = new MyV8Handler(this);
  CefRefPtr<CefV8Value> func =
      CefV8Value::CreateFunction("setMessageCallback", v8_handler);

  app->SetValue("setMessageCallback", func, V8_PROPERTY_ATTRIBUTE_NONE);

  context->Exit();

  CefRefPtr<CefProcessMessage> ipc_dicc_req =
      CefProcessMessage::Create("ipc_dicc_req");

  struct test a = {.msg = "123456789\0", .id = 2, .test = 2.2};
  CefRefPtr<CefListValue> response_args = ipc_dicc_req->GetArgumentList();

  CefRefPtr<CefBinaryValue> msg =
      CefBinaryValue::Create(&a, sizeof(struct test));
  response_args->SetBinary(sizeof(struct test), msg);

  frame->SendProcessMessage(PID_BROWSER, ipc_dicc_req);
}

void SetList(CefRefPtr<CefListValue> source, CefRefPtr<CefV8Value> target) {
  for (size_t i = 0; i < source->GetSize(); ++i) {
    CefValueType type = source->GetType(i);
    switch (type) {
    case VTYPE_BOOL:
      target->SetValue(i, CefV8Value::CreateBool(source->GetBool(i)));
      break;
    case VTYPE_INT:
      target->SetValue(i, CefV8Value::CreateInt(source->GetInt(i)));
      break;
    case VTYPE_DOUBLE:
      target->SetValue(i, CefV8Value::CreateDouble(source->GetDouble(i)));
      break;
    case VTYPE_STRING:
      target->SetValue(i, CefV8Value::CreateString(source->GetString(i)));
      break;
    case VTYPE_LIST: {
      CefRefPtr<CefListValue> list = source->GetList(i);
      CefRefPtr<CefV8Value> value = CefV8Value::CreateArray(list->GetSize());
      SetList(list, value);
      target->SetValue(i, value);
      break;
    }
    default:
      target->SetValue(i, CefV8Value::CreateNull());
      break;
    }
  }
}

bool MyRenderProcessHandler::OnProcessMessageReceived(
    CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
    CefProcessId source_process, CefRefPtr<CefProcessMessage> message) {

  bool handled = false;

  if (!callback_map_.empty()) {
    const CefString &message_name = message->GetName();
    auto key =
        std::make_pair(message_name.ToString(), browser->GetIdentifier());
    auto it = callback_map_.find(key);

    if (it != callback_map_.end()) {
      CefRefPtr<CefV8Context> context = it->second.first;

      // 1. Verify context validity BEFORE touching V8 handles
      if (context.get() && context->IsValid()) {

        // 2. CRITICAL: Enter the context BEFORE referencing/copying the
        // CefV8Value!
        context->Enter();

        CefRefPtr<CefV8Value> callback = it->second.second;

        if (callback.get() && callback->IsValid()) {
          CefV8ValueList arguments;
          arguments.push_back(CefV8Value::CreateString(message_name));

          CefRefPtr<CefListValue> list = message->GetArgumentList();
          CefRefPtr<CefV8Value> args = CefV8Value::CreateArray(list->GetSize());

          SetList(list, args);
          arguments.push_back(args);

          // Execute callback
          CefRefPtr<CefV8Value> retval =
              callback->ExecuteFunction(nullptr, arguments);

          if (retval.get() && retval->IsBool()) {
            handled = retval->GetBoolValue();
          } else {
            handled = true;
          }
        }

        // 3. Exit the context
        context->Exit();

      } else {
        // Context is no longer valid, erase entry safely
        callback_map_.erase(it);
      }
    }
  }

  return handled;
}
void MyRenderProcessHandler::OnContextReleased(
    CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
    CefRefPtr<CefV8Context> context) {
  for (auto it = callback_map_.begin(); it != callback_map_.end();) {
    if (it->second.first->IsSame(context)) {
      it = callback_map_.erase(it);
    } else {
      ++it;
    }
  }
}
