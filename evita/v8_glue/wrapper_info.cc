// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/v8_glue/wrapper_info.h"

#include "base/bind.h"
#include "base/logging.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/per_isolate_data.h"
#include "evita/v8_glue/scriptable.h"

namespace v8_glue {

namespace {
// Changes behavior of constructor call for singleton:
//  - Type() => throw "Cannot be called as function"
//  - new Type() => throw "Cannot use with new operator" if not creating
//    wrapper
void ConstructorCallbackForSingletonClass(
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

void ConstructorCallbackForAbstractClass(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  auto const isolate = info.GetIsolate();
  isolate->ThrowException(
    // TODO(yosi) Provie abstract name in error message.
    gin::StringToV8(isolate, "Cannot create instance for abstract class"));
}

}  // namespace

WrapperInfo::WrapperInfo(const char* class_name)
    : embedder_(gin::kEmbedderEvita),
      class_name_(class_name) {
}

WrapperInfo* WrapperInfo::inherit_from() const {
  return nullptr;
}

const char* WrapperInfo::singleton_name() const {
  return nullptr;
}

AbstractScriptable* WrapperInfo::singleton() const {
  return nullptr;
}

v8::Handle<v8::FunctionTemplate> WrapperInfo::CreateConstructorTemplate(
    v8::Isolate* isolate) {
  auto const templ = v8::FunctionTemplate::New(isolate);
  if (singleton_name())
    templ->SetCallHandler(ConstructorCallbackForSingletonClass);
  else
    templ->SetCallHandler(ConstructorCallbackForAbstractClass);
  return templ;
}

gin::ObjectTemplateBuilder WrapperInfo::CreateInstanceTemplateBuilder(
    v8::Isolate* isolate) {
  if (!class_name())
    return gin::ObjectTemplateBuilder(isolate);
  auto class_templ = GetOrCreateConstructorTemplate(isolate);
  gin::ObjectTemplateBuilder builder(isolate, class_templ,
                                     class_templ->PrototypeTemplate());
  SetupInstanceTemplate(builder);
  return builder;
}

WrapperInfo* WrapperInfo::From(v8::Handle<v8::Object> object) {
  if (object->InternalFieldCount() != gin::kNumberOfInternalFields)
    return nullptr;
  auto const info = static_cast<WrapperInfo*>(
      object->GetAlignedPointerFromInternalField(gin::kWrapperInfoIndex));
  return info->embedder() == gin::kEmbedderEvita ? info : nullptr;
}

// Get or create FunctionTemplate for constructor.
v8::Handle<v8::FunctionTemplate> WrapperInfo::GetOrCreateConstructorTemplate(
    v8::Isolate* isolate) {
  auto const data = gin::PerIsolateData::From(isolate);
  auto present = data->GetFunctionTemplate(gin_wrapper_info());
  if (!present.IsEmpty())
    return present;

  auto const templ = CreateConstructorTemplate(isolate);
  CHECK(!templ.IsEmpty());
  data->SetFunctionTemplate(gin_wrapper_info(), templ);

  templ->SetClassName(gin::StringToSymbol(isolate, class_name()));

  // We must set internal field count for instance template before creating
  // function object.
  templ->InstanceTemplate()->
    SetInternalFieldCount(gin::kNumberOfInternalFields);

  if (auto const base_info = inherit_from()) {
    auto base_templ = data->GetFunctionTemplate(base_info->gin_wrapper_info());
    DCHECK(!base_templ.IsEmpty()) << "No such base class "
        << base_info->class_name();
    templ->Inherit(base_templ);
  }

  // Install instance template
  GetOrCreateInstanceTemplate(isolate);

  return templ;
}

v8::Handle<v8::ObjectTemplate> WrapperInfo::GetOrCreateInstanceTemplate(
    v8::Isolate* isolate) {
  auto const data = gin::PerIsolateData::From(isolate);
  auto present = data->GetObjectTemplate(gin_wrapper_info());
  if (!present.IsEmpty())
    return present;

  auto const templ = CreateInstanceTemplateBuilder(isolate).Build();
  CHECK(!templ.IsEmpty());
  data->SetObjectTemplate(gin_wrapper_info(), templ);
  return templ;
}

void WrapperInfo::Install(v8::Isolate* isolate,
                          v8::Handle<v8::ObjectTemplate> global) {
  auto constructor = GetOrCreateConstructorTemplate(isolate)->GetFunction();
  global->Set(constructor->GetName()->ToString(), constructor);
  if (auto const singleton = this->singleton()) {
    global->Set(gin::StringToV8(isolate, singleton_name()),
                singleton->GetWrapper(isolate));
  }
}

void WrapperInfo::SetupInstanceTemplate(ObjectTemplateBuilder&) {
}

}  // namespace v8_glue
