// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_OS_FILE_STATUS_H_
#define EVITA_DOM_OS_FILE_STATUS_H_

#include "evita/dom/public/io_callback.h"
#include "evita/v8_glue/converter.h"

namespace domapi {
struct FileStatus;
}

namespace gin {
template <>
struct Converter<domapi::FileStatus> {
  static v8::Local<v8::Value> ToV8(v8::Isolate* isolate,
                                   const domapi::FileStatus& data);
};
}  // namespace gin

#endif  // EVITA_DOM_OS_FILE_STATUS_H_
