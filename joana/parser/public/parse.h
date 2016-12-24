// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PARSER_PUBLIC_PARSE_H_
#define JOANA_PARSER_PUBLIC_PARSE_H_

#include "base/macros.h"
#include "joana/parser/public/parser_context.h"
#include "joana/parser/public/parser_export.h"

namespace joana {

namespace ast {
class Node;
}

class SourceCodeRange;

//
// ParserOptions
//
struct JOANA_PARSER_EXPORT ParserOptions {
  // Disable automatic semicolon insertion.
  bool disable_automatic_semicolon = false;

  // When |enable_strict_backslash| is true, a character after backslash
  // not '"\bfnrtv are error.
  bool enable_strict_backslash = false;

  // When |enable_strict_regex| is true, RegExp syntax character should be
  // escaped.
  bool enable_strict_regexp = false;
};

//
// ParserOptionsBuilder
//
class JOANA_PARSER_EXPORT ParserOptionsBuilder final {
 public:
  ParserOptionsBuilder();
  ~ParserOptionsBuilder();

  ParserOptions Build();

  ParserOptionsBuilder& SetAutomaticSemicolon(bool value);
  ParserOptionsBuilder& SetStrictBackslash(bool value);
  ParserOptionsBuilder& SetStrictRegExp(bool value);

 private:
  ParserOptions options_;

  DISALLOW_COPY_AND_ASSIGN(ParserOptionsBuilder);
};

//
// The parser entry point.
//
JOANA_PARSER_EXPORT const ast::Node& Parse(ParserContext* context,
                                           const SourceCodeRange& range,
                                           const ParserOptions& options);

}  // namespace joana

#endif  // JOANA_PARSER_PUBLIC_PARSE_H_
