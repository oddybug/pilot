#include "render_handler.h"

#include <cstring>
#include <string>

#include "data/hashmap.h"
#include "data/hashmap_helpers.h"
#include "log.h"

#include "include/base/cef_logging.h"
#include "include/internal/cef_ptr.h"
#include "include/internal/cef_string.h"

#include "ui.h"
#include "ui_msg_common.h"
#include "ui_msg_render.h"

class MyRenderProcessHandler;

MyRenderProcessHandler *g_instance = nullptr;

MyRenderProcessHandler *MyRenderProcessHandler::GetInstance() {
  return g_instance;
};

bool MyV8Handler::Execute(const CefString &name, CefRefPtr<CefV8Value> object,
                          const CefV8ValueList &arguments,
                          CefRefPtr<CefV8Value> &retval, CefString &exception) {
  // TODO: Theres two maps. it could be just one. Not quite but. To think

  CefRefPtr<CefV8Context> context = CefV8Context::GetCurrentContext();
  if (!(context.get() && context->IsValid())) {
    WARN("[INTERNAL ERROR] V8 context invalid");
    return false;
  }
  if (!render_handler_.get()) {
    WARN("[INTERNAL ERROR] render handler not initialized");
    return false;
  }

  if (name == "PullMessage") {
    if (arguments.size() < 1) {
      WARN("not enough arguments passed for 'PullMessage'");
      return false;
    }
    if (!arguments[0]->IsString()) {
      WARN("first argument of 'PullMessage' has to be a string "
           "identifier");
      return false;
    }

    std::string message_name = arguments[0]->GetStringValue().ToString();

    map_T map = ui_msg_render_pull_m();
    struct pull_msg_e_render *e;
    e = (struct pull_msg_e_render *)gen_map_find(map, message_name.c_str());
    if (!e) {
      ERROR("No call binded with key: %s", message_name.c_str());
      return false;
    }

    s32 browser_id = context->GetBrowser()->GetIdentifier();
    MyRenderProcessHandler::CallbackKey key =
        std::make_pair(message_name, browser_id);

    context->Enter();
    render_handler_->pull_callback_map_[key] =
        std::make_pair(context, arguments[1]);
    context->Exit();

    CefRefPtr<CefProcessMessage> msg = render_handler_->CreateMessage(
        message_name.c_str(), e, ConvertV8ListToCefList(arguments));
    context->GetFrame()->SendProcessMessage(PID_BROWSER, msg);
    return true;

  } else if (name == "SetPushClbk") {
    INFO("SetPushClbk");
    if (arguments.size() < 2) {
      WARN("not enough arguments passed for 'setPushClbk'");
      return false;
    }
    if (!arguments[0]->IsString()) {
      WARN("first argument of 'setPushClbk' has to be a string "
           "identifier");
      return false;
    }
    if (!arguments[1]->IsFunction()) {
      WARN("Second argument of 'setPushClbk' has to be a callback function"
           "identifier");
      return false;
    }

    std::string message_name = arguments[0]->GetStringValue().ToString();
    ERROR("message_name");

    if (gen_map_find(render_handler_->msg_push_m_,
                     (void *)message_name.c_str())) {
      WARN("Redefinition of push message: %s", message_name.c_str());
      return false;
    }

    s32 browser_id = context->GetBrowser()->GetIdentifier();
    MyRenderProcessHandler::CallbackKey key =
        std::make_pair(message_name, browser_id);
    ERROR("MAP KEY: %s - %d", key.first.c_str(), key.second);

    context->Enter();
    render_handler_->push_callback_map_[key] =
        std::make_pair(context, arguments[1]);
    context->Exit();

    CefRefPtr<CefV8Value> func = arguments[1];
    u32 f_args = 0;
    CefRefPtr<CefV8Value> f_av = func->GetValue("length");
    if (f_av) {
      f_args = f_av->GetIntValue();
    }
    enum ARG_TYPE at = {U32};
    struct args args = {.args = &at, .n_args = 1};
    msg_T req = ui_msg_push_request(message_name.c_str(), &args);
    ui_msg_push_u32(req, f_args);
    CefRefPtr<CefBinaryValue> bs_req =
        CefBinaryValue::Create(ui_msg_bitstream(req), ui_msg_size(req));

    CefRefPtr<CefProcessMessage> cef_msg =
        CefProcessMessage::Create("push_msg_req");
    CefRefPtr<CefListValue> cef_args = cef_msg->GetArgumentList();
    cef_args->SetBinary(0, bs_req);

    context->GetFrame()->SendProcessMessage(PID_BROWSER, cef_msg);
    ui_msg_free(req);

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

MyRenderProcessHandler::MyRenderProcessHandler() {
  DCHECK(!g_instance);
  g_instance = this;

  init_msg_pull_m_();
  init_msg_push_m();
}

MyRenderProcessHandler::~MyRenderProcessHandler() { g_instance = nullptr; };

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
  CefRefPtr<CefV8Value> pull_func =
      CefV8Value::CreateFunction("PullMessage", v8_handler);

  CefRefPtr<CefV8Value> push_func =
      CefV8Value::CreateFunction("SetPushClbk", v8_handler);

  app->SetValue("PullMessage", pull_func, V8_PROPERTY_ATTRIBUTE_NONE);
  app->SetValue("SetPushClbk", push_func, V8_PROPERTY_ATTRIBUTE_NONE);

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

  bool handled = false;
  CefString message_name = message->GetName();

  if (message_name == "ipc_dicc_stream") {
    CefRefPtr<CefListValue> args = message->GetArgumentList();
    CefRefPtr<CefBinaryValue> bin = args->GetBinary(0);
    void *data = (void *)bin->GetRawData();

    ui_msg_pull_rm_add(data, bin->GetSize());

    // ui_ipc_stream_insert(e_map_, data); DEPRECATED
  } else if (message_name == "entry_response") {
    if (pull_callback_map_.empty())
      return false;

    CefRefPtr<CefListValue> args = message->GetArgumentList();
    CefRefPtr<CefBinaryValue> bs = args->GetBinary(0);
    INFO("data_size: %d", bs->GetSize());
    void *stream = (void *)bs->GetRawData();
    c8 *sc = (c8 *)stream;
    c8 en[strlen(sc) + 1];
    strcpy(en, sc);

    std::string en_s = en;
    CallbackKey key = std::make_pair(en_s, browser->GetIdentifier());
    auto it = pull_callback_map_.find(key);

    if (it == pull_callback_map_.end()) {
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
    struct pull_msg_e_render *e =
        (struct pull_msg_e_render *)gen_map_find(msg_pull_m_, en);

    if (!e) {
      WARN("No associated entry with name: %s", en);
      return false;
    }

    struct args out_args = e->out;
    msg_T msg = ui_msg_get_fs(en, &out_args);

    CefRefPtr<CefV8Value> callback = it->second.second;

    if (!(callback.get() && callback->IsValid())) {
      return false; // handled = false;
    }

    CefV8ValueList arguments;
    for (u8 i = 0; i < e->out.n_args; i++) {

      s32 value;
      ui_msg_arg_read(msg, &value);

      INFO("%d", e->out.args[i]);
      INFO("asdw value: %d", value);
      PushArgument(arguments, &value, e->out.args[i]);
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
    pull_callback_map_.erase(it);
  } else if (message_name == "push_msg_req") {
    CefRefPtr<CefBinaryValue> bs = message->GetArgumentList()->GetBinary(0);
    bs->GetSize();
    c8 sc[bs->GetSize()];
    void *stream = (void *)sc;
    bs->GetData(stream, bs->GetSize(), 0);

    msg_T msg = ui_msg_get_fs_raw(stream, bs->GetSize());

    c8 *msg_name = (c8 *)malloc(sizeof(c8 *) * (strlen(sc) + 1));
    if (!msg_name) {
      ERROR("failed to allocate memory. errno: %d", errno);
      return false;
    }
    ui_msg_cpy_name(msg, msg_name);

    s32 valid;

    std::string m_s = msg_name;
    CallbackKey key = std::make_pair(msg_name, browser->GetIdentifier());

    INFO("IT ENTRY NAME %s - %d", key.first.c_str(), key.second);
    auto it = push_callback_map_.find(key);
    if (it == push_callback_map_.end()) {
      WARN("'%s' was not registered in the push callback map", msg_name);
      return true;
    }

    ui_msg_read_s32_r(msg, &valid);
    if (valid == -1) {
      push_callback_map_.erase(it);
      return true;
    }
    s32 n_args = valid;
    INFO("n_args: push %d", valid);

    struct args a;
    enum ARG_TYPE *at = (enum ARG_TYPE *)malloc(sizeof(enum ARG_TYPE) * n_args);
    if (!at) {
      ERROR("failed to allocate memory");
      return false;
    }
    s32 i;
    for (i = 0; i < n_args; i++) {
      enum ARG_TYPE et;
      ui_msg_read_s32_r(msg, (s32 *)&et);
      at[i] = et;
    }
    a.n_args = n_args;
    a.args = at;

    struct push_msg_e_render *pmsge =
        (struct push_msg_e_render *)malloc(sizeof(struct push_msg_e_render));
    if (!pmsge) {
      ERROR("failed to allocate memory");
      return false;
    }

    INFO("INSERTED");

    pmsge->out = a;
    gen_map_insert(msg_push_m_, msg_name, pmsge);

    return true;
  } else if (message_name == "push_msg") {
    if (push_callback_map_.empty())
      return false;

    CefRefPtr<CefListValue> args = message->GetArgumentList();
    CefRefPtr<CefBinaryValue> bs = args->GetBinary(0);
    INFO("data_size: %d", bs->GetSize());
    void *stream = (void *)bs->GetRawData();
    c8 *sc = (c8 *)stream;
    c8 en[strlen(sc) + 1];
    strcpy(en, sc);

    std::string en_s = en;
    CallbackKey key = std::make_pair(en_s, browser->GetIdentifier());
    auto it = push_callback_map_.find(key);

    if (it == push_callback_map_.end()) {
      WARN("No associated push entry with name: %s", en);
      return false;
    }
    CefRefPtr<CefV8Context> context = it->second.first;

    // 1. Verify context validity BEFORE touching V8 handles
    if (!(context.get() && context->IsValid())) {
      WARN("[INTERNAL ERROR] V8 context invalid");
      return false;
    }
    context->Enter();
    // sc += strlen(en) + sizeof(c8);
    struct push_msg_e_render *e =
        (struct push_msg_e_render *)gen_map_find(msg_push_m_, en);

    if (!e) {
      WARN("No associated entry with name: %s", en);
      return false;
    }

    struct args out_args = e->out;
    WARN("out args; %d", out_args.n_args);
    msg_T msg = ui_msg_get_fs(en, &out_args);

    CefRefPtr<CefV8Value> callback = it->second.second;

    if (!(callback.get() && callback->IsValid())) {
      return false; // handled = false;
    }

    WARN("HOLA");
    CefV8ValueList arguments;
    for (u8 i = 0; i < e->out.n_args; i++) {

      // WRONG SO WRONG
      s32 value[1];
      ui_msg_arg_read(msg, value);

      INFO("%d", e->out.args[i]);
      INFO("herewebo: %d", value);
      PushArgument(arguments, &value, e->out.args[i]);
    }

    WARN("HOLA");
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
    // pull_callback_map_.erase(it);
  }

  return handled;
}

void MyRenderProcessHandler::OnContextReleased(
    CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
    CefRefPtr<CefV8Context> context) {
  for (auto it = pull_callback_map_.begin(); it != pull_callback_map_.end();) {
    if (it->second.first->IsSame(context)) {
      it = pull_callback_map_.erase(it);
    } else {
      ++it;
    }
  }
}

map_T MyRenderProcessHandler::GetPullMsgMap() { return msg_pull_m_; };

static void pilot_pull_entry_free_item_fn_(struct item_T *item);

static void pilot_pull_entry_free_item_fn_(struct item_T *item) {
  struct pull_msg_e_render *value = (pull_msg_e_render *)item->value;
  ui_msg_pullme_free(value);
};

static void pilot_push_entry_free_item_fn_(struct item_T *item);

static void pilot_push_entry_free_item_fn_(struct item_T *item) {
  struct push_msg_e_render *value = (push_msg_e_render *)item->value;
  ui_msg_pushme_free(value);
};

void MyRenderProcessHandler::init_msg_pull_m_() {
  msg_pull_m_ =
      gen_map_create(DICC_SIZE, gen_map_hash_fn_c8p, gen_map_cmp_key_c8p,
                     pilot_pull_entry_free_item_fn_);
};

void MyRenderProcessHandler::init_msg_push_m() {
  msg_push_m_ =
      gen_map_create(DICC_SIZE, gen_map_hash_fn_c8p, gen_map_cmp_key_c8p,
                     pilot_push_entry_free_item_fn_);
};

// TODO: A LOT OF MORE CHECKING IS NEEDED
bool MyRenderProcessHandler::CheckType(enum ARG_TYPE c_type,
                                       CefValueType js_type) {

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
CefRefPtr<CefProcessMessage> MyRenderProcessHandler::CreateMessage(
    const c8 *name, struct pull_msg_e_render *e, CefRefPtr<CefListValue> args) {

  struct args in = e->in;

  CefRefPtr<CefListValue> args_cpy = args->Copy();
  args_cpy->Remove(0);
  if (e->out.n_args > 0)
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
    // TODO: Could provide more info. Could not read type X expected Y. WE DO
    // NOW BUT NEED TO CHECK
    ERROR("Wrong arg types provided in call '%s'", name);
    return nullptr;
  }

  size_t name_len = strlen(name) + sizeof(c8);
  size_t bs_size = name_len + ui_args_argsv_get(&e->in);

  msg_T msg = ui_msg_pull_render_create((c8 *)name);

  INFO("name: %s", ui_msg_bitstream(msg));

  CreateMessageBs(msg, name, &in, args_cpy);

  CefRefPtr<CefProcessMessage> cef_msg = CefProcessMessage::Create("entry");
  CefRefPtr<CefListValue> args_msg = cef_msg->GetArgumentList();
  CefRefPtr<CefBinaryValue> msg_bs =
      CefBinaryValue::Create(ui_msg_bitstream(msg), bs_size);
  args_msg->SetBinary(0, msg_bs);

  ui_msg_free(msg);
  return cef_msg;
}

void MyRenderProcessHandler::CreateMessageBs(msg_T msg, const c8 *name,
                                             struct args *in,
                                             CefRefPtr<CefListValue> &args) {
  for (int i = 0; i < in->n_args; i++) {
    CefRefPtr<CefValue> value = args->GetValue(i);
    CopyValueToStream(value, msg);
  }
}

void MyRenderProcessHandler::CopyValueToStream(CefRefPtr<CefValue> &value,
                                               msg_T msg) {
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
    ui_msg_push_s32(msg, v);
    INFO("value: %d", v);
    // ui_ipc_
    // ui_ipc_stream_write_arg(stream, &v, S32);
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
                                          void *value, enum ARG_TYPE type) {
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
