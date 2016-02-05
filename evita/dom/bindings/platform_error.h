// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_BINDINGS_PLATFORM_ERROR_H_
#define EVITA_DOM_BINDINGS_PLATFORM_ERROR_H_

#include <string>

#include "base/strings/string_piece.h"
#include "evita/ginx/converter.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// PlatformError
//
class PlatformError final {
 public:
  PlatformError(base::StringPiece api_name, int error_code);
  ~PlatformError();

  const std::string& api_name() const { return api_name_; }
  int error_code() const { return error_code_; }

 private:
  std::string api_name_;
  int error_code_;
};

}  // namespace dom

namespace gin {
template <>
struct Converter<dom::PlatformError> {
  static v8::Local<v8::Value> ToV8(v8::Isolate* isolate,
                                   const dom::PlatformError& error);
};
}  // namespace gin

#endif  // EVITA_DOM_BINDINGS_PLATFORM_ERROR_H_
