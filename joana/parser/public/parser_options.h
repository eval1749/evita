// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PARSER_PUBLIC_PARSER_OPTIONS_H_
#define JOANA_PARSER_PUBLIC_PARSER_OPTIONS_H_

#include "base/macros.h"
#include "joana/parser/public/parser_export.h"

namespace joana {

// Disable automatic semicolon insertion.

#define FOR_EACH_PARSER_OPTION(V)                                      \
  V(disable_automatic_semicolon, bool, false)                          \
  V(enable_strict_backslash, bool, false,                              \
    "If true, a character after backslash should be one of '\\bfntv'") \
  V(enable_strict_regexp, bool, false,                                 \
    "If true, RegExp syntax characters should be escaped.")

//
// ParserOptions
//
class JOANA_PARSER_EXPORT ParserOptions {
 public:
  class Builder;

  ParserOptions(const ParserOptions& other);
  ParserOptions();
  ~ParserOptions();

#define V(name, type, ...) \
  type name() const { return name##_; }
  FOR_EACH_PARSER_OPTION(V)
#undef V

 private:
#define V(name, type, default, ...) type name##_ = default;
  FOR_EACH_PARSER_OPTION(V)
#undef V
};

}  // namespace joana

#endif  // JOANA_PARSER_PUBLIC_PARSER_OPTIONS_H_
