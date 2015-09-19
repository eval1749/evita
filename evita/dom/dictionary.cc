// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/dictionary.h"

#include "base/strings/stringprintf.h"
#include "evita/dom/script_host.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/runner.h"

namespace dom {

base::string16 V8ToString(v8::Handle<v8::Value> value);

Dictionary::Dictionary() {}

Dictionary::~Dictionary() {}

v8::Isolate* Dictionary::isolate() const {
  return v8::Isolate::GetCurrent();
}

bool Dictionary::Init(v8::Isolate* isolate, v8::Handle<v8::Value> dict) {
  if (dict.IsEmpty())
    return true;
  if (!dict->IsObject())
    return false;
  auto const keys = dict->ToObject()->GetPropertyNames();
  auto const keys_length = keys->Length();
  for (auto index = 0u; index < keys_length; ++index) {
    auto key = keys->Get(index);
    auto const value = dict->ToObject()->Get(key);
    auto const result = HandleKeyValue(key, value);
    switch (result) {
      case HandleResult::CanNotConvert:
        isolate->ThrowException(v8::Exception::TypeError(gin::StringToV8(
            isolate, base::StringPrintf(L"Bad value for %ls",
                                        V8ToString(key).c_str()))));
        return false;
      case HandleResult::NotFound:
        isolate->ThrowException(v8::Exception::TypeError(gin::StringToV8(
            isolate,
            base::StringPrintf(L"Invalid key: %ls", V8ToString(key).c_str()))));
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
