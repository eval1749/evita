// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/v8_glue/constructor_template.h"

#include "base/strings/stringprintf.h"
#include "evita/v8_glue/per_isolate_data.h"
#include "evita/v8_glue/scriptable.h"

namespace v8_glue {
namespace internal {
void FinishConstructCall(const v8::FunctionCallbackInfo<v8::Value>& info,
                         AbstractScriptable* impl) {
  auto wrapper = info.Holder();
  auto wrapper2 = info.This();
  impl->Bind(info.GetIsolate(), wrapper);
  info.GetReturnValue().Set(wrapper);
}

bool IsValidConstructCall(const v8::FunctionCallbackInfo<v8::Value>& info,
                          int arity) {
  auto const isolate = info.GetIsolate();
  if (!info.IsConstructCall()) {
    isolate->ThrowException(gin::StringToV8(isolate,
        "Cannot be called as function."));
    return false;
  }
  // When we're creating wrapper, e.g. Scriptable::GetWrapper(), we have
  // already had Scriptable object.
  if (v8_glue::PerIsolateData::From(isolate)->is_creating_wrapper())
    return false;
  if (arity != info.Length()) {
    isolate->ThrowException(gin::StringToV8(isolate, base::StringPrintf(
        "Expect %d argument(s), but %d argument(s) supplied.",
        arity, info.Length())));
    return false;
  }
  return true;
}
}  // namespace internal
}  // namespace v8_glue
