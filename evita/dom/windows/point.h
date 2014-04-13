// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_point_h)
#define INCLUDE_evita_dom_point_h

#include "evita/dom/converter.h"
#include "evita/dom/public/float_point.h"

namespace gin {
template<>
struct Converter<domapi::FloatPoint> {
  static bool FromV8(v8::Isolate* isolate, v8::Handle<v8::Value> val,
                     domapi::FloatPoint* out);
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
                                    const domapi::FloatPoint& point);
};
} // namespace gin

#endif //!defined(INCLUDE_evita_dom_point_h)
