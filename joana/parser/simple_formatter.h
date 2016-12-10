// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PARSER_SIMPLE_FORMATTER_H_
#define JOANA_PARSER_SIMPLE_FORMATTER_H_

#include <iosfwd>

#include "joana/public/ast/node_visitor.h"

#include "joana/public/ast/node_forward.h"

namespace joana {
namespace internal {

class SimpleFormatter final : public ast::NodeVisitor {
 public:
  explicit SimpleFormatter(std::ostream* ostream);
  ~SimpleFormatter();

  void Format(const ast::Node& node);
  void OutputIndent();
  void OutputAsSourceCode(const ast::Node& node);

 private:
#define V(name) void Visit##name(ast::name* node) final;
  FOR_EACH_CONCRETE_AST_NODE(V)
#undef V

  int indent_ = 0;
  std::ostream* const ostream_;

  DISALLOW_COPY_AND_ASSIGN(SimpleFormatter);
};

}  // namespace internal
}  // namespace joana

#endif  // JOANA_PARSER_SIMPLE_FORMATTER_H_
