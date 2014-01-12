// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_converter_h)
#define INCLUDE_evita_dom_converter_h

#include "evita/v8_glue/converter.h"

namespace gin {

// text::Posn
template<>
struct Converter<text::Posn> {
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
                                    text::Posn position);
  static bool FromV8(v8::Isolate* isolate, v8::Handle<v8::Value> val,
                     text::Posn* out);
};

} // namespace gin

#endif //!defined(INCLUDE_evita_dom_converter_h)
