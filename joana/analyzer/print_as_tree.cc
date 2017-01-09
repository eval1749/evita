// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "joana/analyzer/print_as_tree.h"

#include "base/auto_reset.h"
#include "base/macros.h"
#include "joana/analyzer/context.h"
#include "joana/analyzer/type.h"
#include "joana/analyzer/value.h"
#include "joana/ast/node.h"
#include "joana/ast/node_printer.h"
#include "joana/ast/node_traversal.h"
#include "joana/ast/syntax.h"

namespace joana {
namespace analyzer {

namespace {

// TODO(eval1749): We should share |PrintContext|, |Indent| and
// |UsingSourceCode| with "joana/testing".

//
// PrintContext
//
struct PrintContext {
  int level = 0;
};

//
// Indent
//
struct Indent {
  explicit Indent(const PrintContext& print_context)
      : level(print_context.level) {}
  explicit Indent(int passed_level) : level(passed_level) {}
  int level;
};

std::ostream& operator<<(std::ostream& ostream, const Indent& indent) {
  ostream << std::endl;
  if (indent.level == 0)
    return ostream;
  for (auto counter = 0; counter < indent.level - 1; ++counter)
    ostream << "|  ";
  ostream << "+--";
  return ostream;
}

//
// IndentScope
//
class IndentScope final {
 public:
  explicit IndentScope(PrintContext* print_context)
      : level_holder_(&print_context->level, print_context->level + 1) {}

  ~IndentScope() = default;

 private:
  base::AutoReset<int> level_holder_;

  DISALLOW_COPY_AND_ASSIGN(IndentScope);
};

// Dispatcher
template <typename T>
struct Printable {
  PrintContext* print_context;
  const Context* context;
  const T* node;
};

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<ast::Node>& printable) {
  const auto& context = *printable.context;
  const auto& node = *printable.node;
  auto& print_context = *printable.print_context;
  ostream << node.syntax();
  if (const auto* value = context.TryValueOf(node))
    ostream << " $" << value->class_name() << '@' << value->id();
  if (const auto* type = context.TryTypeOf(node))
    ostream << " {" << *type << '}';
  if (node.arity() == 0)
    return ostream << " |" << ast::AsSourceCode(node) << '|';
  IndentScope scope(&print_context);
  for (const auto& child : ast::NodeTraversal::ChildNodesOf(node))
    ostream << Indent(print_context)
            << Printable<ast::Node>{&print_context, &context, &child};
  return ostream;
}

}  // namespace

PrintableTreeNode AsPrintableTree(const Context& context,
                                  const ast::Node& node) {
  return PrintableTreeNode{&context, &node};
}

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableTreeNode& printable) {
  PrintContext print_context;
  const auto& context = *printable.context;
  const auto& node = *printable.node;
  return ostream << Printable<ast::Node>{&print_context, &context, &node};
}

}  // namespace analyzer
}  // namespace joana
