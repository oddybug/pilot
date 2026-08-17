#include "render_handler.h"
#include "data/hashmap.h"
#include "data/hashmap_helpers.h"
#include "include/internal/cef_ptr.h"
#include "log.h"
#include "ui_ipc.h"
#include <cstring>

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

MyRenderProcessHandler::MyRenderProcessHandler() { init_e_map(); }

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

  const std::string message_name = message->GetName();
  INFO("Renderer message name: %s", message_name.c_str());

  if (message_name == "ipc_dicc_stream") {
    CefRefPtr<CefListValue> args = message->GetArgumentList();
    CefRefPtr<CefBinaryValue> bin = args->GetBinary(0);
    void *data = (void *)bin->GetRawData();
    ui_ipc_stream_insert(e_map_, data);
  };

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

static void pilot_entry_free_entry_c_(struct entry_c_T *e);

static void pilot_entry_free_entry_c_(struct entry_c_T *e) {
  free(e->e.in_args.args);
  free(e->e.out_args.args);
};

static void pilot_entry_free_item_fn_(struct item_T *item);

static void pilot_entry_free_item_fn_(struct item_T *item) {
  struct entry_c_T *value = (entry_c_T *)item->value;
  pilot_entry_free_entry_c_(value);
};

void MyRenderProcessHandler::init_e_map() {
  e_map_ = gen_map_create(DICC_SIZE, gen_map_hash_fn_c8p, gen_map_cmp_key_c8p,
                          pilot_entry_free_item_fn_);
};

// TODO: A LOT OF MORE CHECKING IS NEEDED
bool MyRenderProcessHandler::CheckType(ARG_TYPE c_type, CefValueType js_type) {

  switch (js_type) {
  case VTYPE_INVALID:
    return false;
    break;
  case VTYPE_NULL:
    return false;
    break;
  case VTYPE_BOOL:
    return false;
    break;
  case VTYPE_INT:
    // more checking
    return c_type == S32 ? true : false;
    break;
  case VTYPE_DOUBLE:
    return false;
    break;
  case VTYPE_STRING:
    return false;
    break;
  case VTYPE_BINARY:
    return false;
    break;
  case VTYPE_DICTIONARY:
    return false;
    break;
  case VTYPE_LIST:
    return false;
    break;

  case VTYPE_NUM_VALUES:
    return false;
    break;
  default:
    return false;
    break;
  }
  return true;
};
CefRefPtr<CefProcessMessage>
MyRenderProcessHandler::CreateMessage(const c8 *name, struct entry_T *e,
                                      CefRefPtr<CefListValue> args) {

  struct args_T in = e->in_args;

  CefRefPtr<CefListValue> args_cpy = args->Copy();
  args_cpy->Remove(0);
  if (e->out_args.n_args > 0)
    args_cpy->Remove(0);

  if (in.n_args > args->GetSize()) {
    ERROR("Not enough args provided in '%s'", name);
    return nullptr;
  }

  bool valid = true;
  int i = 0;
  while (valid and i < in.n_args) {
    valid = CheckType(in.args[i], args->GetValue(i)->GetType());
    i++;
  }
  if (!valid) {
    // TODO: Could provide more info. Could not read type X expected Y
    ERROR("Wrong arg types provided in call '%s'", name);
    return nullptr;
  }

  size_t name_len = strlen(name) + sizeof(c8);
  size_t bs_size = name_len + ui_ipc_argsv_get(&e->in_args);

  void *bs = malloc(bs_size);

  if (!bs) {
    ERROR("failed to allocate memory");
    return nullptr;
  }

  CreateMessageBs(bs, name, &in, args_cpy);

  CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("entry");
  CefRefPtr<CefListValue> args_msg = msg->GetArgumentList();
  CefRefPtr<CefBinaryValue> msg_bs = CefBinaryValue::Create(bs, bs_size);

  free(bs);
  return nullptr;
}

void MyRenderProcessHandler::CreateMessageBs(void *stream, const c8 *name,
                                             struct args_T *in,
                                             CefRefPtr<CefListValue> args) {
  size_t name_len = strlen(name) + sizeof(c8);

  uint8_t *s_cpy = static_cast<u8 *>(stream);
  strcpy(reinterpret_cast<c8 *>(s_cpy), name);
  s_cpy += name_len;

  for (int i = 0; i < in->n_args; i++) {
    CefRefPtr<CefValue> value = args->GetValue(i);
    void *bin = (void *)value->GetBinary()->GetRawData();

    ui_ipc_stream_copy_arg(reinterpret_cast<void **>(&s_cpy), bin, in->args[i]);
  }
}
