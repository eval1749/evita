// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/bindings/platform_error.h"

#include "evita/dom/v8_strings.h"
#include "evita/v8_glue/runner.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// PlatformError
//
PlatformError::PlatformError(base::StringPiece api_name, int error_code)
    : api_name_(api_name.as_string()), error_code_(error_code) {}

PlatformError::~PlatformError() {}

}  // namespace dom

namespace gin {
v8::Local<v8::Value> Converter<dom::PlatformError>::ToV8(
    v8::Isolate* isolate,
    const dom::PlatformError& error) {
  const auto runner = v8_glue::Runner::From(isolate);
  const auto constructor = runner->global()
                               ->Get(dom::v8Strings::PlatformError.Get(isolate))
                               ->ToObject();
  return runner->CallAsConstructor(
      constructor, gin::ConvertToV8(isolate, error.api_name()),
      gin::ConvertToV8(isolate, error.error_code()));
}
}  // namespace gin
