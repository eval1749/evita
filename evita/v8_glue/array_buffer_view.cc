// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/v8_glue/array_buffer_view.h"

namespace gin {

bool Converter<ArrayBufferView*>::FromV8(v8::Isolate* isolate,
                                         v8::Handle<v8::Value> val,
                                         ArrayBufferView** out) {
  if (!val->IsArrayBufferView())
    return false;
  *out = new ArrayBufferView(isolate,
                             v8::Handle<v8::ArrayBufferView>::Cast(val));
  return true;
}

}  // namespace gin
