// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/converter.h"

#include "evita/v8_glue/converter.h"

namespace gin {

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
