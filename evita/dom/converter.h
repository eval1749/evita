// Copyright (c) 1996-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_CONVERTER_H_
#define EVITA_DOM_CONVERTER_H_

#include "evita/text/offset.h"
#include "evita/v8_glue/converter.h"

namespace base {
class Time;
}

namespace text {
struct LineAndColumn;
class Offset;
}

namespace gin {

template <>
struct Converter<base::Time> {
  static v8::Local<v8::Value> ToV8(v8::Isolate* isolate, base::Time file_time);
  static bool FromV8(v8::Isolate* isolate,
                     v8::Local<v8::Value> val,
                     base::Time* out);
};

template <>
struct Converter<text::LineAndColumn> {
  static v8::Local<v8::Value> ToV8(v8::Isolate* isolate,
                                   const text::LineAndColumn& line_and_column);
};

template <>
struct Converter<text::Offset> {
  static bool FromV8(v8::Isolate* isolate,
                     v8::Local<v8::Value> val,
                     text::Offset* out);
  static v8::Local<v8::Value> ToV8(v8::Isolate* isolate,
                                   const text::Offset& line_and_column);
};

}  // namespace gin

#endif  // EVITA_DOM_CONVERTER_H_
