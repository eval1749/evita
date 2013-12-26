// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/v8_glue/script_wrappable.h"

#include "base/logging.h"
#include "evita/v8_glue/per_isolate_data.h"
BEGIN_V8_INCLUDE
#include "gin/converter.h"
#include "gin/object_template_builder.h"
#include "gin/per_isolate_data.h"
END_V8_INCLUDE


namespace v8_glue {

namespace {
void constructorCallBack(const v8::FunctionCallbackInfo<v8::Value>& info) {
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

AbstractScriptWrappable::~AbstractScriptWrappable() {
  wrapper_.Reset();
}

void AbstractScriptWrappable::WeakCallback(
    const v8::WeakCallbackData<v8::Object, AbstractScriptWrappable>& data) {
  auto const wrappable = data.GetParameter();
  wrappable->wrapper_.Reset();
}

v8::Handle<v8::FunctionTemplate> AbstractScriptWrappable::GetFunctionTemplate(
    v8::Isolate* isolate) {
  auto const info = wrapper_info();
  DCHECK(wrapper_class_name());
  auto const data = gin::PerIsolateData::From(isolate);
  auto present = data->GetFunctionTemplate(info);
  if (!present.IsEmpty())
    return present;
  auto templ = v8::FunctionTemplate::New(isolate);
  templ->SetClassName(gin::StringToSymbol(isolate, wrapper_class_name()));
  data->SetFunctionTemplate(info, templ);

  templ->SetCallHandler(constructorCallBack);

  // We must set internal field count for instance template before creating
  // function object.
  templ->InstanceTemplate()->
      SetInternalFieldCount(gin::kNumberOfInternalFields);

  return templ;
}

gin::ObjectTemplateBuilder AbstractScriptWrappable::GetObjectTemplateBuilder(
    v8::Isolate* isolate) {
  if (!wrapper_class_name())
    return gin::ObjectTemplateBuilder(isolate);
  auto templ = GetFunctionTemplate(isolate);
  return gin::ObjectTemplateBuilder(isolate, templ->InstanceTemplate());
}

v8::Handle<v8::Object> AbstractScriptWrappable::GetWrapper(
    v8::Isolate* isolate) {
  auto const info = wrapper_info();
  auto const data = gin::PerIsolateData::From(isolate);
  auto templ = data->GetObjectTemplate(info);
  if (templ.IsEmpty()) {
    templ = GetObjectTemplateBuilder(isolate).Build();
    CHECK(!templ.IsEmpty());
    data->SetObjectTemplate(info, templ);
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
  return wrapper;
}

}  // namespace v8_glue
