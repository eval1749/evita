// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_v8_glue_converter_h)
#define INCLUDE_evita_v8_glue_converter_h

#include "base/strings/string16.h"
BEGIN_V8_INCLUDE
#include "gin/converter.h"
END_V8_INCLUDE

namespace gin {

template<>
struct Converter<base::char16> {
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
                                    base::char16 val);
  static bool FromV8(v8::Isolate* isolate, v8::Handle<v8::Value> val,
                     base::char16* out);
};

template<>
struct Converter<base::string16> {
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
                                    const base::string16& string);
  static bool FromV8(v8::Isolate* isolate, v8::Handle<v8::Value> val,
                     base::string16* out);
};

template<>
struct Converter<text::Posn> {
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
                                    text::Posn position);
  static bool FromV8(v8::Isolate* isolate, v8::Handle<v8::Value> val,
                     text::Posn* out);
};

v8::Handle<v8::Value> StringToV8(v8::Isolate* isolate,
                                  const base::string16& string);

} // namespace gin

#endif //!defined(INCLUDE_evita_v8_glue_converter_h)
