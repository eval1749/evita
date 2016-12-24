// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/parser/public/parser_options_builder.h"

namespace joana {

//
// ParserOptions::Builder
//
ParserOptions::Builder::Builder() = default;
ParserOptions::Builder::~Builder() = default;

#define V(name, type, ...)                                                 \
  ParserOptions::Builder& ParserOptions::Builder::set_##name(type value) { \
    options_.name##_ = value;                                              \
    return *this;                                                          \
  }
FOR_EACH_PARSER_OPTION(V)
#undef V

}  // namespace joana
