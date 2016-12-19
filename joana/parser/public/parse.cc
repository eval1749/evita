// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/parser/public/parse.h"

#include "joana/parser/parser.h"

namespace joana {

using Parser = internal::Parser;

//
// ParserOptionsBuilder
//
ParserOptionsBuilder::ParserOptionsBuilder() = default;
ParserOptionsBuilder::~ParserOptionsBuilder() = default;

ParserOptions ParserOptionsBuilder::Build() {
  return options_;
}

ParserOptionsBuilder& ParserOptionsBuilder::SetAutomaticSemicolon(bool value) {
  options_.disable_automatic_semicolon = !value;
  return *this;
}

ParserOptionsBuilder& ParserOptionsBuilder::SetStrictBackslash(bool value) {
  options_.enable_strict_backslash = value;
  return *this;
}

ParserOptionsBuilder& ParserOptionsBuilder::SetStrictRegExp(bool value) {
  options_.enable_strict_regexp = value;
  return *this;
}

//
// Parse; the entry point
//
const ast::Node& Parse(ast::EditContext* context,
                       const SourceCodeRange& range,
                       const ParserOptions& options) {
  Parser parser(context, range, options);
  return parser.Run();
}

}  // namespace joana
