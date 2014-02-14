// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/converter.h"

#include "base/time/time.h"
#include "evita/v8_glue/converter.h"

namespace gin {

// base::Time
v8::Handle<v8::Value> Converter<base::Time>::ToV8(v8::Isolate* isolate,
                                                  base::Time time) {
  return v8::Date::New(isolate, time.ToJsTime());
}

bool Converter<base::Time>::FromV8(v8::Isolate*, v8::Handle<v8::Value> val,
                                   base::Time* out) {
  if (!val->IsDate())
    return false;
  auto const date = v8::Handle<v8::Date>::Cast(val);
  *out = base::Time::FromJsTime(date->ValueOf());
  return true;
}

// text::Posn
v8::Handle<v8::Value> Converter<text::Posn>::ToV8(v8::Isolate* isolate,
                                                  text::Posn position) {
  return Converter<int>::ToV8(isolate, static_cast<int>(position));
}

bool Converter<text::Posn>::FromV8(v8::Isolate* isolate,
                                   v8::Handle<v8::Value> val,
                                   text::Posn* out) {
  int int_val;
  if (!Converter<int>::FromV8(isolate, val, &int_val))
    return false;
  *out = text::Posn(int_val);
  return true;
}

}  // namespace gin
