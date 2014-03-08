// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_v8_glue_array_buffer_view_h)
#define INCLUDE_evita_v8_glue_array_buffer_view_h

#include "evita/v8_glue/v8.h"
#include "gin/array_buffer.h"

namespace gin {
template<>
struct Converter<ArrayBufferView*> {
  static bool FromV8(v8::Isolate* isolate, v8::Handle<v8::Value> val,
                     ArrayBufferView** out);
};
}  // namespace gin

#endif //!defined(INCLUDE_evita_v8_glue_array_buffer_view_h)
