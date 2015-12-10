// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_OS_IO_ERROR_H_
#define EVITA_DOM_OS_IO_ERROR_H_

#include "evita/dom/public/io_error.h"
#include "evita/v8_glue/converter.h"

namespace gin {
template <>
struct Converter<domapi::IoError> {
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
                                    const domapi::IoError& error);
};
}  // namespace gin

#endif  // EVITA_DOM_OS_IO_ERROR_H_
