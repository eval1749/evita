// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/converter.h"

#include "base/time/time.h"
#include "evita/ginx/converter.h"
#include "evita/text/buffer.h"

namespace gin {

// base::Time
v8::Local<v8::Value> Converter<base::Time>::ToV8(v8::Isolate* isolate,
                                                 base::Time time) {
  return v8::Date::New(isolate, time.ToJsTime());
}

bool Converter<base::Time>::FromV8(v8::Isolate*,
                                   v8::Local<v8::Value> val,
                                   base::Time* out) {
  if (!val->IsDate())
    return false;
  auto const date = v8::Local<v8::Date>::Cast(val);
  *out = base::Time::FromJsTime(date->ValueOf());
  return true;
}

v8::Local<v8::Value> Converter<text::LineAndColumn>::ToV8(
    v8::Isolate* isolate,
    const text::LineAndColumn& line_and_column) {
  auto const result = v8::Object::New(isolate);
  result->Set(gin::StringToV8(isolate, "column"),
              v8::Integer::New(isolate, line_and_column.column));
  result->Set(gin::StringToV8(isolate, "lineNumber"),
              v8::Integer::New(isolate, line_and_column.line_number));
  return result;
}

bool Converter<text::Offset>::FromV8(v8::Isolate* isolate,
                                     v8::Local<v8::Value> val,
                                     text::Offset* out) {
  int value;
  if (!ConvertFromV8(isolate, val, &value))
    return false;
  if (value < 0)
    return false;
  *out = text::Offset(value);
  return true;
}

v8::Local<v8::Value> Converter<text::Offset>::ToV8(v8::Isolate* isolate,
                                                   const text::Offset& offset) {
  return v8::Integer::New(isolate, offset.value()).As<v8::Value>();
}

v8::Local<v8::Value> Converter<v8::Local<v8::Map>>::ToV8(
    v8::Isolate* isolate,
    v8::Local<v8::Map> val) {
  return val.As<v8::Value>();
}

bool Converter<v8::Local<v8::Map>>::FromV8(v8::Isolate* isolate,
                                           v8::Local<v8::Value> val,
                                           v8::Local<v8::Map>* out) {
  if (!val->IsMap())
    return false;
  *out = v8::Local<v8::Map>::Cast(val);
  return true;
}

}  // namespace gin
