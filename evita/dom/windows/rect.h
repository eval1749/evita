// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_DOM_WINDOWS_RECT_H_
#define EVITA_DOM_WINDOWS_RECT_H_

#include "evita/dom/converter.h"
#include "evita/dom/public/float_rect.h"

namespace gin {
template <>
struct Converter<domapi::FloatRect> {
  static bool FromV8(v8::Isolate* isolate,
                     v8::Local<v8::Value> val,
                     domapi::FloatRect* out);
  static v8::Local<v8::Value> ToV8(v8::Isolate* isolate,
                                   const domapi::FloatRect& rect);
};
}  // namespace gin

#endif  // EVITA_DOM_WINDOWS_RECT_H_
