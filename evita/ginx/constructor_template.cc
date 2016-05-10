// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/ginx/constructor_template.h"

#include "base/strings/stringprintf.h"
#include "evita/ginx/converter.h"
#include "evita/ginx/per_isolate_data.h"
#include "evita/ginx/scriptable.h"

namespace ginx {
namespace internal {

base::string16 AsString(v8::Local<v8::Value> value) {
  v8::String::Value string_value(value);
  if (!string_value.length())
    return base::string16();
  return base::string16(reinterpret_cast<base::char16*>(*string_value),
                        static_cast<size_t>(string_value.length()));
}

void FinishConstructCall(const v8::FunctionCallbackInfo<v8::Value>& info,
                         AbstractScriptable* impl) {
  if (!impl) {
    // Constructor may throw an exception.
    return;
  }
  const auto& wrapper = info.Holder();
  impl->Bind(info.GetIsolate(), wrapper);

  const auto& context = info.GetIsolate()->GetCurrentContext();
  // TODO(eval1749): Should we have string "prototype" in another place?
  const auto& prototype_str = gin::StringToV8(info.GetIsolate(), "prototype");
  const auto& prototype =
      info.NewTarget().As<v8::Function>()->Get(prototype_str);
  const auto& couldSetPrototype = wrapper->SetPrototype(context, prototype);
  if (couldSetPrototype.IsNothing()) {
    DVLOG(0) << "SetPrototype failed";
    return;
  }
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
