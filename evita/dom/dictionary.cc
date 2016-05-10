// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/dictionary.h"

#include "base/strings/stringprintf.h"
#include "evita/ginx/converter.h"
#include "evita/ginx/runner.h"

namespace dom {

Dictionary::Dictionary() {}

Dictionary::~Dictionary() {}

v8::Isolate* Dictionary::isolate() const {
  return v8::Isolate::GetCurrent();
}

bool Dictionary::Init(v8::Isolate* isolate, v8::Local<v8::Value> dict) {
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
            isolate, base::StringPrintf("Bad value for %s",
                                        *v8::String::Utf8Value(key)))));
        return false;
      case HandleResult::NotFound:
        isolate->ThrowException(v8::Exception::TypeError(gin::StringToV8(
            isolate, base::StringPrintf("Invalid key: %s",
                                        *v8::String::Utf8Value(key)))));
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
