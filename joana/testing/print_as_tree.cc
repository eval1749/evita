// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "base/auto_reset.h"
#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "base/strings/utf_string_conversions.h"
#include "joana/ast/bindings.h"
#include "joana/ast/expressions.h"
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
  }                                                                      \
  std::ostream& operator<<(std::ostream& ostream,                        \
                           const Printable##name& printable);
FOR_EACH_AST_BINDING_ELEMENT(V)
FOR_EACH_AST_JSDOC(V)
FOR_EACH_AST_TOKEN(V)
FOR_EACH_AST_TYPE(V)
#undef V

// Binding
std::ostream& operator<<(std::ostream& ostream,
                         const PrintableArrayBindingPattern& printable) {
  auto& context = *printable.context;
  const auto& node = *printable.node;
  ostream << "#array_pattern";
  IndentScope scope(&context);
  {
    for (const auto& element : node.elements())
      ostream << Indent(context) << AsPrintable(&context, element);
  }
  if (node.initializer().Is<ast::ElisionExpression>())
    return ostream;
  return ostream << Indent(context)
                 << AsPrintable(&context, node.initializer());
}

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableBindingCommaElement& printable) {
  return ostream << "#binding_comma";
}

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableBindingInvalidElement& printable) {
  return ostream << "#binding_invalid";
}

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableBindingNameElement& printable) {
  auto& context = *printable.context;
  const auto& node = *printable.node;
  ostream << "#binding_name " << AsPrintable(&context, node.name());
  if (node.initializer().Is<ast::ElisionExpression>())
    return ostream;
  IndentScope scope(&context);
  return ostream << Indent(context)
                 << AsPrintable(&context, node.initializer());
}

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableBindingProperty& printable) {
  auto& context = *printable.context;
  const auto& node = *printable.node;
  ostream << "#binding_property " << AsPrintable(&context, node.name());
  IndentScope scope(&context);
  return ostream << Indent(context) << AsPrintable(&context, node.element());
}

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableBindingRestElement& printable) {
  auto& context = *printable.context;
  const auto& node = *printable.node;
  ostream << "#binding_rest";
  IndentScope scope(&context);
  return ostream << Indent(context) << AsPrintable(&context, node.element());
}

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableObjectBindingPattern& printable) {
  auto& context = *printable.context;
  const auto& node = *printable.node;
  ostream << "#object_pattern";
  IndentScope scope(&context);
  {
    for (const auto& element : node.elements())
      ostream << Indent(context) << AsPrintable(&context, element);
  }
  if (node.initializer().Is<ast::ElisionExpression>())
    return ostream;
  return ostream << Indent(context)
                 << AsPrintable(&context, node.initializer());
}

// JsDoc
std::ostream& operator<<(std::ostream& ostream,
                         const PrintableJsDocDocument& printable) {
  auto& context = *printable.context;
  const auto& node = *printable.node;
  ostream << "#document";
  IndentScope scope(&context);
  for (const auto& element : node.elements())
    ostream << Indent(context) << AsPrintable(&context, element);
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableJsDocName& printable) {
  auto& context = *printable.context;
  const auto& node = *printable.node;
  return ostream << "#name " << AsPrintable(&context, node.name());
}

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableJsDocTag& printable) {
  auto& context = *printable.context;
  const auto& node = *printable.node;
  ostream << AsPrintable(&context, node.name());
  IndentScope scope(&context);
  for (const auto& parameter : node.parameters())
    ostream << Indent(context) << AsPrintable(&context, parameter);
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableJsDocText& printable) {
  const auto& node = *printable.node;
  return ostream << '|' << AsPrintable(node.range()) << '|';
}

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableJsDocType& printable) {
  auto& context = *printable.context;
  const auto& node = *printable.node;
  return ostream << "#type " << AsPrintable(&context, node.type());
}

// Tokens
std::ostream& operator<<(std::ostream& ostream,
                         const PrintableComment& printable) {
  const auto& node = *printable.node;
  return ostream << "#comment |" << AsPrintable(node.range()) << '|';
}

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableEmpty& printable) {
  return ostream << "#empty";
}

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableJsDoc& printable) {
  auto& context = *printable.context;
  const auto& node = *printable.node;
  ostream << "#jsdoc";
  IndentScope scope(&context);
  return ostream << AsPrintable(&context, node.document());
}

std::ostream& operator<<(std::ostream& ostream,
                         const PrintablePunctuator& printable) {
  const auto& node = *printable.node;
  return ostream << AsPrintable(node.range());
}

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableName& printable) {
  const auto& node = *printable.node;
  return ostream << AsPrintable(node.range());
}

// Types
std::ostream& operator<<(std::ostream& ostream,
                         const PrintableAnyType& printable) {
  return ostream << '*';
}

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableFunctionType& printable) {
  auto& context = *printable.context;
  const auto& node = *printable.node;
  ostream << "function " << static_cast<int>(node.kind());
  IndentScope scope(&context);
  {
    IndentScope parameters_scope(&context);
    for (const auto& parameter_type : node.parameter_types())
      ostream << Indent(context) << AsPrintable(&context, parameter_type);
  }
  return ostream << Indent(context)
                 << AsPrintable(&context, node.return_type());
}

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableInvalidType& printable) {
  return ostream << "(invalid)";
}

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableNullableType& printable) {
  auto& context = *printable.context;
  const auto& node = *printable.node;
  return ostream << '?' << AsPrintable(&context, node.type());
}

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableNonNullableType& printable) {
  auto& context = *printable.context;
  const auto& node = *printable.node;
  return ostream << '!' << AsPrintable(&context, node.type());
}

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableOptionalType& printable) {
  auto& context = *printable.context;
  const auto& node = *printable.node;
  return ostream << '=' << AsPrintable(&context, node.type());
}

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableRecordType& printable) {
  auto& context = *printable.context;
  const auto& node = *printable.node;
  ostream << "record";
  IndentScope scope(&context);
  for (const auto& member : node.members()) {
    ostream << Indent(context) << AsPrintable(&context, *member.first) << ": "
            << AsPrintable(&context, *member.second);
  }
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableRestType& printable) {
  auto& context = *printable.context;
  const auto& node = *printable.node;
  return ostream << "..." << AsPrintable(&context, node.type());
}

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableTupleType& printable) {
  auto& context = *printable.context;
  const auto& node = *printable.node;
  ostream << "tuple";
  IndentScope scope(&context);
  for (const auto& member : node.members())
    ostream << Indent(context) << AsPrintable(&context, member);
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableTypeApplication& printable) {
  auto& context = *printable.context;
  const auto& node = *printable.node;
  ostream << "application " << AsPrintable(&context, node.name());
  IndentScope scope(&context);
  for (const auto& parameter : node.parameters())
    ostream << Indent(context) << AsPrintable(&context, parameter);
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableTypeGroup& printable) {
  auto& context = *printable.context;
  const auto& node = *printable.node;
  ostream << "group";
  IndentScope scope(&context);
  return ostream << Indent(context) << AsPrintable(&context, node.type());
}

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableTypeName& printable) {
  auto& context = *printable.context;
  const auto& node = *printable.node;
  return ostream << AsPrintable(&context, node.name());
}

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableUnionType& printable) {
  auto& context = *printable.context;
  const auto& node = *printable.node;
  ostream << "union";
  IndentScope scope(&context);
  for (const auto& member : node.members())
    ostream << Indent(context) << AsPrintable(&context, member);
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableUnknownType& printable) {
  return ostream << '?';
}

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableVoidType& printable) {
  return ostream << "void";
}

// Dispatcher
std::ostream& operator<<(std::ostream& ostream,
                         const PrintableNode& printable) {
  auto& context = *printable.context;
  const auto& node = *printable.node;
#define V(name)             \
  if (node.Is<ast::name>()) \
    return ostream << Printable##name{&context, &node.As<ast::name>()};
  FOR_EACH_AST_BINDING_ELEMENT(V)
  FOR_EACH_AST_JSDOC(V)
  FOR_EACH_AST_TOKEN(V)
  FOR_EACH_AST_TYPE(V)
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
