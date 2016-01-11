// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/v8_glue/scriptable.h"

#include "base/logging.h"
#include "evita/gc/collector.h"
#include "evita/v8_glue/object_template_builder.h"
#include "evita/v8_glue/per_isolate_data.h"

namespace v8_glue {

AbstractScriptable::AbstractScriptable() {}

AbstractScriptable::~AbstractScriptable() {
  wrapper_.Reset();
}

void AbstractScriptable::Bind(v8::Isolate* isolate,
                              v8::Local<v8::Object> wrapper) {
  auto const count = wrapper->InternalFieldCount();
  CHECK_EQ(gin::kNumberOfInternalFields, count);
  wrapper->SetAlignedPointerInInternalField(gin::kWrapperInfoIndex,
                                            wrapper_info());
  wrapper->SetAlignedPointerInInternalField(gin::kEncodedValueIndex, this);
  wrapper_.Reset(isolate, wrapper);
  wrapper_.SetWeak(this, WeakCallback);
  gc::Collector::instance()->AddToRootSet(this);
}

v8::Local<v8::Object> AbstractScriptable::GetWrapper(
    v8::Isolate* isolate) const {
  DCHECK(this);
  if (!wrapper_.IsEmpty())
    return v8::Local<v8::Object>::New(isolate, wrapper_);
  auto ctor =
      wrapper_info()->GetOrCreateConstructorTemplate(isolate)->GetFunction();
  ConstructorModeScope constructor_mode_scope(isolate, kWrapExistingObject);
  auto const wrapper = ctor->NewInstance();
  CHECK(!wrapper.IsEmpty());
  const_cast<AbstractScriptable*>(this)->Bind(isolate, wrapper);
  return wrapper;
}

void AbstractScriptable::WeakCallback(
    const v8::WeakCallbackData<v8::Object, AbstractScriptable>& data) {
  auto const wrappable = data.GetParameter();
  wrappable->wrapper_.Reset();
  gc::Collector::instance()->RemoveFromRootSet(wrappable);
}

namespace internal {

void* FromV8Impl(v8::Isolate*,
                 v8::Local<v8::Value> val,
                 WrapperInfo* wrapper_info) {
  if (!val->IsObject())
    return nullptr;
  auto obj = v8::Local<v8::Object>::Cast(val);
  auto const info = WrapperInfo::From(obj);

  // If this fails, the object is not managed by Gin. It is either a normal JS
  // object that's not wrapping any external C++ object, or it is wrapping
  // some C++ object, but that object isn't managed by Gin (maybe Blink).
  if (!info)
    return nullptr;

  // If this fails, the object is managed by Gin, but it's not wrapping an
  // instance of the C++ class associated with wrapper_info.
  for (auto runner = info; runner; runner = runner->inherit_from()) {
    if (runner == wrapper_info)
      return obj->GetAlignedPointerFromInternalField(gin::kEncodedValueIndex);
  }
  return nullptr;
}

}  // namespace internal

}  // namespace v8_glue
