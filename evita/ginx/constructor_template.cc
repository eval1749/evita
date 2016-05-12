// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/ginx/constructor_template.h"

#include "base/strings/stringprintf.h"
#include "evita/ginx/converter.h"
#include "evita/ginx/per_isolate_data.h"
#include "evita/ginx/scriptable.h"

namespace ginx {
namespace internal {

// This function is used in callback functions generated from IDL compiler.
void FinishConstructCall(const v8::FunctionCallbackInfo<v8::Value>& info,
                         AbstractScriptable* impl) {
  if (!impl) {
    // Constructor may throw an exception.
    return;
  }
  impl->Bind(info.GetIsolate(), info.Holder());
}

// This function is used in callback functions generated from IDL compiler.
bool IsValidConstructCall(const v8::FunctionCallbackInfo<v8::Value>& info) {
  auto const isolate = info.GetIsolate();
  if (!info.IsConstructCall()) {
    isolate->ThrowException(
        gin::StringToV8(isolate, "Cannot be called as function."));
    return false;
  }
  // When we're creating wrapper, e.g. Scriptable::GetWrapper(), we have
  // already had Scriptable object.
  return !ginx::PerIsolateData::From(isolate)->is_creating_wrapper();
}
}  // namespace internal
}  // namespace ginx
