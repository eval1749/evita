// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/windows/point.h"

#include "v8_strings.h"  // NOLINT(build/include)

namespace gin {

bool Converter<domapi::FloatPoint>::FromV8(v8::Isolate* isolate,
                                           v8::Local<v8::Value> val,
                                           domapi::FloatPoint* out) {
  if (val.IsEmpty() || !val->IsObject())
    return false;
  auto const obj = val->ToObject();
  if (!obj->GetConstructorName()->Equals(dom::v8Strings::Point.Get(isolate)))
    return false;
  float x;
  if (!ConvertFromV8(isolate, obj->Get(dom::v8Strings::x.Get(isolate)), &x))
    return false;
  float y;
  if (!ConvertFromV8(isolate, obj->Get(dom::v8Strings::y.Get(isolate)), &y))
    return false;
  *out = domapi::FloatPoint(x, y);
  return true;
}

v8::Local<v8::Value> Converter<domapi::FloatPoint>::ToV8(
    v8::Isolate* isolate,
    const domapi::FloatPoint& point) {
  auto const context = isolate->GetCurrentContext();
  auto const point_ctor =
      context->Global()->Get(dom::v8Strings::Point.Get(isolate));
  v8::Local<v8::Value> argv[]{ConvertToV8(isolate, point.x()),
                              ConvertToV8(isolate, point.y())};
  return point_ctor->ToObject()->CallAsConstructor(arraysize(argv), argv);
}

}  // namespace gin
