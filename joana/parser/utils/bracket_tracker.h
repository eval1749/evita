// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PARSER_UTILS_BRACKET_TRACKER_H_
#define JOANA_PARSER_UTILS_BRACKET_TRACKER_H_

#include <stack>
#include <utility>
#include <vector>

#include "base/macros.h"

namespace joana {
class ErrorSink;
class SourceCodeRange;

namespace ast {
class Node;
enum class PunctuatorKind;
}

namespace parser {

//
// BracketTracker
//
class BracketTracker final {
 public:
  struct Description {
    ast::PunctuatorKind open;
    int open_error;
    ast::PunctuatorKind close;
    int close_error;
  };

  BracketTracker(ErrorSink* error_sink,
                 const SourceCodeRange& range,
                 const std::vector<Description>& descriptions);
  ~BracketTracker();

  void Feed(const ast::Node& token);
  void Finish();

 private:
  void Check(const ast::Node& token,
             ast::PunctuatorKind expected,
             int error_code);
  void Mark(const ast::Node& token, int error_code);

  const std::vector<Description> descriptions_;
  ErrorSink& error_sink_;
  const SourceCodeRange& source_code_range_;
  const std::pair<ast::PunctuatorKind, ast::PunctuatorKind> min_max_;
  std::stack<std::pair<const ast::Node*, int>> stack_;

  DISALLOW_COPY_AND_ASSIGN(BracketTracker);
};

}  // namespace parser
}  // namespace joana

#endif  // JOANA_PARSER_UTILS_BRACKET_TRACKER_H_
