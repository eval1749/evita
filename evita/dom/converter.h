// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_DOM_CONVERTER_H_
#define EVITA_DOM_CONVERTER_H_

#include "evita/precomp.h"
#include "evita/v8_glue/converter.h"

namespace base {
class Time;
}

namespace text {
struct LineAndColumn;
}

namespace gin {

template <>
struct Converter<base::Time> {
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate, base::Time file_time);
  static bool FromV8(v8::Isolate* isolate,
                     v8::Handle<v8::Value> val,
                     base::Time* out);
};


template <>
struct Converter<text::LineAndColumn> {
  static v8::Handle<v8::Value> ToV8(
      v8::Isolate* isolate,
      const text::LineAndColumn& line_and_column);
};

}  // namespace gin

#endif  // EVITA_DOM_CONVERTER_H_
