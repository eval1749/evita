// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/ginx/constructor_template.h"

#include "base/strings/stringprintf.h"
#include "evita/ginx/per_isolate_data.h"
#include "evita/ginx/scriptable.h"

namespace ginx {
namespace internal {
void FinishConstructCall(const v8::FunctionCallbackInfo<v8::Value>& info,
                         AbstractScriptable* impl) {
  auto wrapper = info.Holder();
  auto wrapper2 = info.This();
  if (!impl) {
    // Constructor may throw an exception.
    return;
  }
  impl->Bind(info.GetIsolate(), wrapper);
  info.GetReturnValue().Set(wrapper);
}

bool IsValidConstructCall(const v8::FunctionCallbackInfo<v8::Value>& info) {
  auto const isolate = info.GetIsolate();
  if (!info.IsConstructCall()) {
    isolate->ThrowException(
        gin::StringToV8(isolate, "Cannot be called as function."));
    return false;
  }
  // When we're creating wrapper, e.g. Scriptable::GetWrapper(), we have
  // already had Scriptable object.
  if (ginx::PerIsolateData::From(isolate)->is_creating_wrapper())
    return false;
  return true;
}
}  // namespace internal
}  // namespace ginx
