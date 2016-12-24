// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PARSER_PUBLIC_PARSER_OPTIONS_BUILDER_H_
#define JOANA_PARSER_PUBLIC_PARSER_OPTIONS_BUILDER_H_

#include "joana/parser/public/parser_options.h"

namespace joana {

//
// ParserOptions::Builder
//
class JOANA_PARSER_EXPORT ParserOptions::Builder final {
 public:
  Builder();
  ~Builder();

  ParserOptions Build() const { return options_; }

#define V(name, type, ...) Builder& set_##name(type new_value);
  FOR_EACH_PARSER_OPTION(V)
#undef V

 private:
  ParserOptions options_;

  DISALLOW_COPY_AND_ASSIGN(Builder);
};

}  // namespace joana

#endif  // JOANA_PARSER_PUBLIC_PARSER_OPTIONS_BUILDER_H_
