// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "base/auto_reset.h"
#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "base/strings/utf_string_conversions.h"
#include "joana/ast/jsdoc_nodes.h"
#include "joana/ast/node.h"
#include "joana/ast/node_factory.h"
#include "joana/ast/node_forward.h"
#include "joana/ast/tokens.h"
#include "joana/ast/types.h"
#include "joana/testing/print_as_tree.h"

namespace joana {
namespace internal {

namespace {

struct Context {
  int level = 0;
};

struct Indent {
  explicit Indent(const Context& context) : level(context.level) {}
  explicit Indent(int passed_level) : level(passed_level) {}
  int level;
};

std::ostream& operator<<(std::ostream& ostream, const Indent& indent) {
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
  explicit IndentScope(Context* context)
      : auto_reset_(&context->level, context->level + 1) {}

  ~IndentScope() = default;

 private:
  base::AutoReset<int> auto_reset_;

  DISALLOW_COPY_AND_ASSIGN(IndentScope);
};

struct PrintableSourceCodeRange {
  const SourceCodeRange* range;
};

PrintableSourceCodeRange AsPrintable(const SourceCodeRange& range) {
  return PrintableSourceCodeRange{&range};
}

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableSourceCodeRange& printable) {
  return ostream << base::UTF16ToUTF8(printable.range->GetString());
}

struct PrintableNode {
  Context* context;
  const ast::Node* node;
};

std::ostream& operator<<(std::ostream& ostream, const PrintableNode& printable);

PrintableNode AsPrintable(Context* context, const ast::Node& node) {
  return PrintableNode{context, &node};
}

#define V(name)                                                          \
  struct Printable##name {                                               \
    Context* context;                                                    \
    const ast::name* node;                                               \
  };                                                                     \
  Printable##name AsPrintable(Context* context, const ast::name& node) { \
    return Printable##name{context, &node};                              \
  }
FOR_EACH_AST_JSDOC(V)
#undef V

// JsDoc
std::ostream& operator<<(std::ostream& ostream,
                         const PrintableJsDocDocument& printable) {
  auto& context = *printable.context;
  const auto& node = *printable.node;
  for (const auto& element : node.elements())
    ostream << AsPrintable(&context, element);
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableJsDocName& printable) {
  auto& context = *printable.context;
  const auto& node = *printable.node;
  return ostream << AsPrintable(&context, node.name());
}

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableJsDocTag& printable) {
  auto& context = *printable.context;
  const auto& node = *printable.node;
  ostream << AsPrintable(&context, node.name()) << std::endl;
  IndentScope scope(&context);
  for (const auto& parameter : node.parameters())
    ostream << Indent(context) << AsPrintable(&context, parameter) << std::endl;
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableJsDocText& printable) {
  const auto& node = *printable.node;
  return ostream << '|' << AsPrintable(node.range()) << '|';
}

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableJsDocType& printable) {
  const auto& node = *printable.node;
  return ostream << AsPrintable(node.type().range());
}

// Dispatcher
std::ostream& operator<<(std::ostream& ostream,
                         const PrintableNode& printable) {
  Context context;
  const auto& node = *printable.node;
#define V(name)             \
  if (node.Is<ast::name>()) \
    return ostream << Printable##name{&context, &node.As<ast::name>()};
  FOR_EACH_AST_JSDOC(V)
#undef V
  return ostream << PrintableSourceCodeRange{&node.range()};
}

}  // namespace

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableAstNode& printable) {
  Context context;
  return ostream << PrintableNode{&context, printable.node};
}

}  // namespace internal

internal::PrintableAstNode AsPrintableTree(const ast::Node& node) {
  return internal::PrintableAstNode{&node};
}

}  // namespace joana
