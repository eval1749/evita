// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

// L1 C4191: 'operator/operation' : unsafe conversion from 'type of
// expression' to 'type required'
#pragma warning(push)
#pragma warning(disable : 4191)

#include "evita/v8_glue/wrapper_info.h"

#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/per_isolate_data.h"
#include "evita/v8_glue/scriptable.h"

namespace v8_glue {

namespace {
// A callback for no constructor class:
//  - Type() => throw "Cannot be called as function"
//  - new Type() => throw "Cannot use with new operator" if not creating
//    wrapper
void ConstructorCallbackForNoConstructor(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  auto const isolate = info.GetIsolate();
  if (!info.IsConstructCall()) {
    isolate->ThrowException(
        gin::StringToV8(isolate, "Cannot be called as function"));
    return;
  }

  if (!PerIsolateData::From(isolate)->is_creating_wrapper()) {
    isolate->ThrowException(
        gin::StringToV8(isolate, "Cannot use with new operator"));
    return;
  }
}

base::string16 V8ToString(v8::Handle<v8::Value> value) {
  v8::String::Value string_value(value);
  if (!string_value.length())
    return base::string16();
  return base::string16(reinterpret_cast<base::char16*>(*string_value),
                        static_cast<size_t>(string_value.length()));
}
}  // namespace

WrapperInfo::WrapperInfo(const char* class_name)
    : embedder_(gin::kEmbedderEvita), class_name_(class_name) {}

WrapperInfo* WrapperInfo::inherit_from() const {
  return nullptr;
}

bool WrapperInfo::is_descendant_or_self_of(const WrapperInfo* other) const {
  for (auto runner = this; runner; runner = runner->inherit_from()) {
    if (runner == other)
      return true;
  }
  return false;
}

v8::Handle<v8::FunctionTemplate> WrapperInfo::CreateConstructorTemplate(
    v8::Isolate* isolate) {
  auto const templ = v8::FunctionTemplate::New(isolate);
  templ->SetCallHandler(ConstructorCallbackForNoConstructor);
  return templ;
}

v8::Handle<v8::ObjectTemplate> WrapperInfo::CreateInstanceTemplate(
    v8::Isolate* isolate) {
  if (!class_name())
    return v8::ObjectTemplate::New(isolate);
  auto class_templ = GetOrCreateConstructorTemplate(isolate);
  return SetupInstanceTemplate(isolate, class_templ->PrototypeTemplate());
}

WrapperInfo* WrapperInfo::From(v8::Handle<v8::Object> object) {
  if (object->InternalFieldCount() != gin::kNumberOfInternalFields)
    return nullptr;
  // TODO(eval1749): We get an unexpected object which doesn't have
  // wrapper_info from a function invoked by |v8::V8::RunMicrotasks()|.
  auto const value = object->GetInternalField(gin::kWrapperInfoIndex);
  if (!value->IsInt32())
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
  templ->InstanceTemplate()->SetInternalFieldCount(
      gin::kNumberOfInternalFields);

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

  auto const templ = CreateInstanceTemplate(isolate);
  CHECK(!templ.IsEmpty());
  data->SetObjectTemplate(gin_wrapper_info(), templ);
  return templ;
}

v8::Handle<v8::FunctionTemplate> WrapperInfo::Install(
    v8::Isolate* isolate,
    v8::Handle<v8::ObjectTemplate> global) {
  auto constructor = GetOrCreateConstructorTemplate(isolate);
  global->Set(gin::StringToV8(isolate, class_name_), constructor);
  return constructor;
}

v8::Handle<v8::ObjectTemplate> WrapperInfo::SetupInstanceTemplate(
    v8::Isolate*,
    v8::Handle<v8::ObjectTemplate> templ) {
  return templ;
}

void WrapperInfo::ThrowArityError(v8::Isolate* isolate,
                                  int min_arity,
                                  int max_arity,
                                  int actual_arity) {
  if (min_arity == max_arity) {
    isolate->ThrowException(gin::StringToV8(
        isolate, base::StringPrintf("Expect %d arguments, but %d supplied",
                                    min_arity, actual_arity)));
    return;
  }
  isolate->ThrowException(gin::StringToV8(
      isolate, base::StringPrintf("Expect %d to %d arguments, but %d supplied",
                                  min_arity, max_arity, actual_arity)));
}

void WrapperInfo::ThrowArgumentError(v8::Isolate* isolate,
                                     const char* expected_type,
                                     v8::Handle<v8::Value> value,
                                     int index) {
  isolate->ThrowException(v8::Exception::TypeError(gin::StringToV8(
      isolate, base::StringPrintf("Expect argument[%d] as %s but %ls", index,
                                  expected_type, V8ToString(value).c_str()))));
}

void WrapperInfo::ThrowReceiverError(v8::Isolate* isolate,
                                     const char* expected_type,
                                     v8::Handle<v8::Value> value) {
  isolate->ThrowException(v8::Exception::TypeError(gin::StringToV8(
      isolate, base::StringPrintf("Expect receiver as %s but %ls.",
                                  expected_type, V8ToString(value).c_str()))));
}

}  // namespace v8_glue

#pragma warning(pop)
