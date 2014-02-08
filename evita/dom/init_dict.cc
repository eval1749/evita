// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/init_dict.h"

#include "base/strings/stringprintf.h"
#include "evita/dom/script_controller.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/runner.h"

namespace dom {

base::string16 V8ToString(v8::Handle<v8::Value> value);

InitDict::InitDict() {
}

InitDict::~InitDict() {
}

v8::Isolate* InitDict::isolate() const {
  return v8::Isolate::GetCurrent();
}

bool InitDict::Init(v8::Handle<v8::Object> dict) {
  if (dict.IsEmpty())
    return true;
  auto const runner = ScriptController::instance()->runner();
  v8_glue::Runner::Scope runner_scope(runner);
  auto const isolate = runner->isolate();

  auto const keys = dict->GetPropertyNames();
  auto const keys_length = keys->Length();
  for (auto index = 0u; index < keys_length; ++index) {
    auto key = keys->Get(index);
    auto const value = dict->Get(key);
    auto const result = HandleKeyValue(key, value);
    switch (result) {
      case HandleResult::CanNotConvert:
        runner->isolate()->ThrowException(v8::Exception::TypeError(
            gin::StringToV8(isolate, base::StringPrintf(L"Bad value for %ls",
                V8ToString(key).c_str()))));
        return false;
      case HandleResult::NotFound:
        runner->isolate()->ThrowException(v8::Exception::TypeError(
            gin::StringToV8(isolate, base::StringPrintf(L"Invalid key: %ls",
                V8ToString(key).c_str()))));
        return false;
      case HandleResult::Succeeded:
        break;
      default:
        NOTREACHED();
        break;
    }
  }
  return true;
}

}  // namespace dom
