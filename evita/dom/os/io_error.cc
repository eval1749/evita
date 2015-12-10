// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/os/io_error.h"

#include "evita/v8_glue/runner.h"
#include "v8_strings.h"  // NOLINT(build/include)

namespace gin {
v8::Handle<v8::Value> Converter<domapi::IoError>::ToV8(
    v8::Isolate* isolate,
    const domapi::IoError& error) {
  auto const runner = v8_glue::Runner::current_runner(isolate);
  auto const os_file_error_ctor = runner->global()
                                      ->Get(dom::v8Strings::Os.Get(isolate))
                                      ->ToObject()
                                      ->Get(dom::v8Strings::File.Get(isolate))
                                      ->ToObject()
                                      ->Get(dom::v8Strings::Error.Get(isolate));
  return runner->CallAsConstructor(os_file_error_ctor,
                                   v8::Integer::New(isolate, error.error_code));
}
}  // namespace gin
