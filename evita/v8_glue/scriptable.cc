// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/v8_glue/scriptable.h"

#include "base/logging.h"
#include "evita/gc/collector.h"
#include "evita/v8_glue/per_isolate_data.h"
BEGIN_V8_INCLUDE
#include "gin/converter.h"
#include "gin/object_template_builder.h"
#include "gin/per_isolate_data.h"
END_V8_INCLUDE


namespace v8_glue {

namespace {
// Changes behavior of constructor call for singleton:
//  - Type() => throw "Cannot be called as function"
//  - new Type() => throw "Cannot use with new operator" if not creating
//    wrapper
void ConstructorCallHandlerForBlocking(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  auto const isolate = info.GetIsolate();
  if (!info.IsConstructCall()) {
     isolate->ThrowException(
        gin::StringToV8(isolate, "Cannot be called as function"));
     return;
  }

  if (!PerIsolateData::From(isolate)->is_creating_wrapper()) {
    isolate->ThrowException(gin::StringToV8(isolate,
        "Cannot use with new operator"));
    return;
  }
}

}  // namespace

AbstractScriptable::~AbstractScriptable() {
  wrapper_.Reset();
}

gin::ObjectTemplateBuilder AbstractScriptable::GetObjectTemplateBuilder(
    v8::Isolate* isolate) {
  auto const info = wrapper_info();
  if (!info->class_name())
    return gin::ObjectTemplateBuilder(isolate);
  auto templ = StaticGetFunctionTemplate(isolate, info);
  return gin::ObjectTemplateBuilder(isolate, templ->InstanceTemplate());
}

v8::Handle<v8::Object> AbstractScriptable::GetWrapper(
    v8::Isolate* isolate) {
  if (!wrapper_.IsEmpty())
    return v8::Local<v8::Object>::New(isolate, wrapper_);
  auto const info = wrapper_info();
  auto const data = gin::PerIsolateData::From(isolate);
  auto templ = data->GetObjectTemplate(info->gin_wrapper_info());
  if (templ.IsEmpty()) {
    templ = GetObjectTemplateBuilder(isolate).Build();
    CHECK(!templ.IsEmpty());
    data->SetObjectTemplate(info->gin_wrapper_info(), templ);
  }
  CHECK_EQ(gin::kNumberOfInternalFields, templ->InternalFieldCount());
  ConstructorModeScope constructor_mode_scope(isolate, kWrapExistingObject);
  auto wrapper = templ->NewInstance();
  CHECK(!wrapper.IsEmpty());
  auto const count = wrapper->InternalFieldCount();
  CHECK_EQ(gin::kNumberOfInternalFields, count);
  wrapper->SetAlignedPointerInInternalField(gin::kWrapperInfoIndex, info);
  wrapper->SetAlignedPointerInInternalField(gin::kEncodedValueIndex, this);
  wrapper_.Reset(isolate, wrapper);
  wrapper_.SetWeak(this, WeakCallback);
  gc::Collector::instance()->AddToRootSet(this);
  return wrapper;
}

v8::Handle<v8::Function> AbstractScriptable::StaticGetConstructor(
    v8::Isolate* isolate, WrapperInfo* info) {
  return StaticGetFunctionTemplate(isolate, info)->GetFunction();
}

v8::Handle<v8::FunctionTemplate>
    AbstractScriptable::StaticGetFunctionTemplate(
        v8::Isolate* isolate, WrapperInfo* info) {
  auto const data = gin::PerIsolateData::From(isolate);
  auto present = data->GetFunctionTemplate(info->gin_wrapper_info());
  if (!present.IsEmpty())
    return present;
  auto templ = v8::FunctionTemplate::New(isolate);
  templ->SetClassName(gin::StringToSymbol(isolate, info->class_name()));
  data->SetFunctionTemplate(info->gin_wrapper_info(), templ);

  if (info->singleton_name())
    templ->SetCallHandler(ConstructorCallHandlerForBlocking);

  // We must set internal field count for instance template before creating
  // function object.
  templ->InstanceTemplate()->
      SetInternalFieldCount(gin::kNumberOfInternalFields);

  if (auto const base_info = info->inherit_from()) {
    auto base_templ = data->GetFunctionTemplate(base_info->gin_wrapper_info());
    DCHECK(!base_templ.IsEmpty()) << "No such base class "
        << base_info->class_name();
    templ->Inherit(base_templ);
  }
  return templ;
}

void AbstractScriptable::WeakCallback(
    const v8::WeakCallbackData<v8::Object, AbstractScriptable>& data) {
  auto const wrappable = data.GetParameter();
  wrappable->wrapper_.Reset();
  gc::Collector::instance()->RemoveFromRootSet(wrappable);
}

namespace internal {

void* FromV8Impl(v8::Isolate*, v8::Handle<v8::Value> val,
                 WrapperInfo* wrapper_info) {
  if (!val->IsObject())
    return nullptr;
  auto obj = v8::Handle<v8::Object>::Cast(val);
  auto const info = WrapperInfo::From(obj);

  // If this fails, the object is not managed by Gin. It is either a normal JS
  // object that's not wrapping any external C++ object, or it is wrapping some
  // C++ object, but that object isn't managed by Gin (maybe Blink).
  if (!info)
    return nullptr;

  // If this fails, the object is managed by Gin, but it's not wrapping an
  // instance of the C++ class associated with wrapper_info.
  if (info != wrapper_info)
    return nullptr;

  return obj->GetAlignedPointerFromInternalField(gin::kEncodedValueIndex);
}

}  // namespace internal

}  // namespace v8_glue
