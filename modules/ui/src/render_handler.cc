#include "render_handler.h"
#include "data/hashmap.h"
#include "data/hashmap_helpers.h"
#include "include/internal/cef_ptr.h"
#include "include/internal/cef_string.h"
#include "log.h"
#include "ui_ipc.h"
#include <cstring>
#include <string>

class MyRenderProcessHandler;

bool MyV8Handler::Execute(const CefString &name, CefRefPtr<CefV8Value> object,
                          const CefV8ValueList &arguments,
                          CefRefPtr<CefV8Value> &retval, CefString &exception) {
  // TODO: Theres two maps. it could be just one. Not quite but. To think

  if (name == "setMessageCallback") {
    if (arguments.size() < 1) {
      WARN("not enough arguments passed for 'setMessageCallback'");
      return false;
    } else if (!arguments[0]->IsString()) {
      WARN("first argument of 'setMessageCallback' has to be a string "
           "identifier");
      return false;
    }
    if (!render_handler_.get()) {
      WARN("[INTERNAL ERROR] render handler not initialized");
      return false;
    }

    // ------ find entry
    std::string message_name = arguments[0]->GetStringValue().ToString();

    struct entry_T *e;
    e = (struct entry_T *)gen_map_find(render_handler_->e_map_,
                                       message_name.c_str());
    if (!e) {
      ERROR("No call binded with key: %s", message_name.c_str());
      return false;
    }
    // ------ Save context
    CefRefPtr<CefV8Context> context = CefV8Context::GetCurrentContext();

    if (!(context.get() && context->IsValid())) {
      WARN("[INTERNAL ERROR] V8 context invalid");
      return false;
    }

    s32 browser_id = context->GetBrowser()->GetIdentifier();

    MyRenderProcessHandler::CallbackKey key =
        std::make_pair(message_name, browser_id);

    context->Enter();
    render_handler_->callback_map_[key] = std::make_pair(context, arguments[1]);
    context->Exit();

    // ------  create message
    CefRefPtr<CefProcessMessage> msg = render_handler_->CreateMessage(
        message_name.c_str(), e, ConvertV8ListToCefList(arguments));
    context->GetFrame()->SendProcessMessage(PID_BROWSER, msg);
    return true;
  }
  return false;
}

#include "include/cef_v8.h"
#include "include/cef_values.h"

CefRefPtr<CefValue>
MyV8Handler::CefV8ValueToCefValue(CefRefPtr<CefV8Value> v8Value) {
  CefRefPtr<CefValue> value = CefValue::Create();

  if (!v8Value.get() || v8Value->IsUndefined() || v8Value->IsNull()) {
    value->SetNull();
  } else if (v8Value->IsBool()) {
    value->SetBool(v8Value->GetBoolValue());
  } else if (v8Value->IsInt() || v8Value->IsUInt()) {
    value->SetInt(v8Value->GetIntValue());
  } else if (v8Value->IsDouble()) {
    value->SetDouble(v8Value->GetDoubleValue());
  } else if (v8Value->IsString()) {
    value->SetString(v8Value->GetStringValue());
  } else if (v8Value->IsArray()) {
    int length = v8Value->GetArrayLength();
    CefRefPtr<CefListValue> list = CefListValue::Create();
    list->SetSize(length);

    for (int i = 0; i < length; ++i) {
      CefRefPtr<CefValue> elem = CefV8ValueToCefValue(v8Value->GetValue(i));
      list->SetValue(i, elem);
    }
    value->SetList(list);
  } else if (v8Value->IsObject()) {
    CefRefPtr<CefDictionaryValue> dict = CefDictionaryValue::Create();
    std::vector<CefString> keys;
    v8Value->GetKeys(keys);

    for (const auto &key : keys) {
      CefRefPtr<CefValue> elem = CefV8ValueToCefValue(v8Value->GetValue(key));
      dict->SetValue(key, elem);
    }
    value->SetDictionary(dict);
  } else {
    value->SetNull();
  }

  return value;
}

CefRefPtr<CefListValue>
MyV8Handler::ConvertV8ListToCefList(const CefV8ValueList &v8List) {
  CefRefPtr<CefListValue> cefList = CefListValue::Create();
  cefList->SetSize(v8List.size());

  for (size_t i = 0; i < v8List.size(); ++i) {
    cefList->SetValue(i, CefV8ValueToCefValue(v8List[i]));
  }

  return cefList;
}

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

  CefString message_name = message->GetName();
  INFO("Renderer message name: %s", message_name.ToString().c_str());

  if (message_name == "ipc_dicc_stream") {
    CefRefPtr<CefListValue> args = message->GetArgumentList();
    CefRefPtr<CefBinaryValue> bin = args->GetBinary(0);
    void *data = (void *)bin->GetRawData();
    ui_ipc_stream_insert(e_map_, data);
  };

  bool handled = false;

  if (message_name == "entry_response") {
    if (!callback_map_.empty()) {

      // INFO("v8 map size: %d", callback_map_.size());
      // INFO("first ele: %s", callback_map_.begin()->first.first.c_str());
      // INFO("second ele: %d", callback_map_.begin()->first.second);
      // INFO("B name: %s", message_name.ToString().c_str());
      // INFO("B ID: %d", browser->GetIdentifier());

      CefRefPtr<CefListValue> args = message->GetArgumentList();
      CefRefPtr<CefBinaryValue> bs = args->GetBinary(0);
      INFO("data_size: %d", bs->GetSize());
      void *stream = (void *)bs->GetRawData();
      c8 *sc = (c8 *)stream;
      c8 en[strlen(sc) + 1];
      strcpy(en, sc);
      std::string en_s = en;
      CallbackKey key = std::make_pair(en_s, browser->GetIdentifier());
      auto it = callback_map_.find(key);

      if (it == callback_map_.end()) {
        WARN("No associated entry with name: %s", en);
        return false;
      }
      CefRefPtr<CefV8Context> context = it->second.first;

      // 1. Verify context validity BEFORE touching V8 handles
      if (!(context.get() && context->IsValid())) {
        WARN("[INTERNAL ERROR] V8 context invalid");
        return false;
      }
      context->Enter();
      sc += strlen(en) + sizeof(c8);
      struct entry_T *e = (struct entry_T *)gen_map_find(e_map_, en);

      if (!e) {
        WARN("No associated entry with name: %s", en);
        return false;
      }

      CefRefPtr<CefV8Value> callback = it->second.second;

      if (!(callback.get() && callback->IsValid())) {
        return false; // handled = false;
      }

      CefV8ValueList arguments;
      for (u8 i = 0; i < e->out_args.n_args; i++) {
        // TODO: pass directly the stream ui_ipc_stream_read_arg
        // is wrong probably
        s32 *value;
        ui_ipc_stream_read_arg((void **)&sc, value, e->out_args.args[i]);
        PushArgument(arguments, value, e->out_args.args[i]);
      }
      // Execute callback
      CefRefPtr<CefV8Value> retval =
          callback->ExecuteFunction(nullptr, arguments);

      if (retval.get() && retval->IsBool()) {

      INFO("RETURN VALUE: %d", retval->GetBoolValue());
        handled = retval->GetBoolValue();
      } else {
        handled = true;
      }

      context->Exit();
      callback_map_.erase(it);
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
    INFO("INVALID");
    return false;
    break;
  case VTYPE_NULL:

    INFO("NULL");
    return false;
    break;
  case VTYPE_BOOL:

    INFO("BOOL");
    return false;
    break;
  case VTYPE_INT:
    INFO("INT");
    // more checking
    return c_type == S32 ? true : false;
    break;
  case VTYPE_DOUBLE:

    INFO("DOUBLE");
    return false;
    break;
  case VTYPE_STRING:
    INFO("STRING");
    return false;
    break;
  case VTYPE_BINARY:
    INFO("BIN");
    return false;
    break;
  case VTYPE_DICTIONARY:
    INFO("DICC");
    return false;
    break;
  case VTYPE_LIST:
    INFO("LIST");
    return false;
    break;

  case VTYPE_NUM_VALUES:
    INFO("NUM VALUES");
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

  if (in.n_args > args_cpy->GetSize()) {
    ERROR("Not enough args provided in '%s'", name);
    return nullptr;
  }

  bool valid = true;
  int i = 0;
  while (valid and i < in.n_args) {
    valid = CheckType(in.args[i], args_cpy->GetValue(i)->GetType());
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
  args_msg->SetBinary(0, msg_bs);

  free(bs);
  return msg;
}

void MyRenderProcessHandler::CreateMessageBs(void *stream, const c8 *name,
                                             struct args_T *in,
                                             CefRefPtr<CefListValue> &args) {
  size_t name_len = strlen(name) + 1;

  c8 *s_cpy = static_cast<c8 *>(stream);
  strcpy(s_cpy, name);
  s_cpy += name_len;
  INFO("OFSET: %d", name_len);

  for (int i = 0; i < in->n_args; i++) {
    CefRefPtr<CefValue> value = args->GetValue(i);
    void *scc = s_cpy;
    CopyValueToStream(value, (void **)&s_cpy);
    INFO("v2 after buffer copy: %d", *(int *)scc);
  }
}

void MyRenderProcessHandler::CopyValueToStream(CefRefPtr<CefValue> &value,
                                               void **stream) {
  CefValueType type = value->GetType();
  switch (type) {
  case VTYPE_INVALID:

    break;
  case VTYPE_NULL:

    break;
  case VTYPE_BOOL:

    break;
  case VTYPE_INT: {
    s32 v = value->GetInt();
    ui_ipc_stream_write_arg(stream, &v, S32);
    break;
  }
  case VTYPE_DOUBLE:
    break;
  case VTYPE_STRING:
    break;
  case VTYPE_BINARY:
    break;
  case VTYPE_DICTIONARY:
    break;
  case VTYPE_LIST:
    break;
  case VTYPE_NUM_VALUES:
    break;
  default:
    break;
  }
};

void MyRenderProcessHandler::PushArgument(CefV8ValueList &arguments,
                                          void *value, ARG_TYPE type) {
  switch (type) {
  case U32:
    arguments.push_back(CefV8Value::CreateUInt(*(u32 *)value));
    break;
  case S32:
    arguments.push_back(CefV8Value::CreateInt(*(s32 *)value));
    break;
  default:
    break;
  }
};
