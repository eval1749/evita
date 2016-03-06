// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_DOM_WINDOWS_POINT_H_
#define EVITA_DOM_WINDOWS_POINT_H_

#include "evita/dom/converter.h"
#include "evita/dom/public/float_point.h"

namespace gin {
template <>
struct Converter<gfx::FloatPoint> {
  static bool FromV8(v8::Isolate* isolate,
                     v8::Local<v8::Value> val,
                     gfx::FloatPoint* out);
  static v8::Local<v8::Value> ToV8(v8::Isolate* isolate,
                                   const gfx::FloatPoint& point);
};
}  // namespace gin

#endif  // EVITA_DOM_WINDOWS_POINT_H_
