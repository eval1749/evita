// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_converter_h)
#define INCLUDE_evita_dom_converter_h

#include "evita/precomp.h"
#include "evita/v8_glue/converter.h"

namespace base {
class Time;
}

namespace gin {

// base::Time
template<>
struct Converter<base::Time> {
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate, base::Time file_time);
  static bool FromV8(v8::Isolate* isolate, v8::Handle<v8::Value> val,
                     base::Time* out);
};

} // namespace gin

#endif //!defined(INCLUDE_evita_dom_converter_h)
