// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>
#include <type_traits>

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

//
// Context
//
struct Context {
  int level = 0;
};

//
// Indent
//
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

//
// UsingSourceCode is a wrapper of AST node to print using source code.
//
struct UsingSourceCode {
  const ast::Node* node;
};

UsingSourceCode SourceCodeOf(const ast::Node& node) {
  return UsingSourceCode{&node};
}

std::ostream& operator<<(std::ostream& ostream,
                         const UsingSourceCode& printable) {
  const auto& node = *printable.node;
  return ostream << base::UTF16ToUTF8(node.range().GetString());
}

//
// Printable
//
template <typename NodeClass>
struct Printable {
  static_assert(std::is_base_of<ast::Node, NodeClass>::value,
                "Should be AST node");
  Context* context;
  const NodeClass* node;
};

Printable<ast::Node> AsPrintable(Context* context, const ast::Node& node) {
  return Printable<ast::Node>{context, &node};
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<ast::Node>& printable);

// Binding
std::ostream& operator<<(std::ostream& ostream,
                         const Printable<ast::ArrayBindingPattern>& printable) {
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
                         const Printable<ast::BindingCommaElement>& printable) {
  return ostream << "#binding_comma";
}

std::ostream& operator<<(
    std::ostream& ostream,
    const Printable<ast::BindingInvalidElement>& printable) {
  return ostream << "#binding_invalid";
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<ast::BindingNameElement>& printable) {
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
                         const Printable<ast::BindingProperty>& printable) {
  auto& context = *printable.context;
  const auto& node = *printable.node;
  ostream << "#binding_property " << AsPrintable(&context, node.name());
  IndentScope scope(&context);
  return ostream << Indent(context) << AsPrintable(&context, node.element());
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<ast::BindingRestElement>& printable) {
  auto& context = *printable.context;
  const auto& node = *printable.node;
  ostream << "#binding_rest";
  IndentScope scope(&context);
  return ostream << Indent(context) << AsPrintable(&context, node.element());
}

std::ostream& operator<<(
    std::ostream& ostream,
    const Printable<ast::ObjectBindingPattern>& printable) {
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
                         const Printable<ast::JsDocDocument>& printable) {
  auto& context = *printable.context;
  const auto& node = *printable.node;
  ostream << "#document";
  IndentScope scope(&context);
  for (const auto& element : node.elements())
    ostream << Indent(context) << AsPrintable(&context, element);
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<ast::JsDocName>& printable) {
  auto& context = *printable.context;
  const auto& node = *printable.node;
  return ostream << "#name " << AsPrintable(&context, node.name());
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<ast::JsDocTag>& printable) {
  auto& context = *printable.context;
  const auto& node = *printable.node;
  ostream << AsPrintable(&context, node.name());
  IndentScope scope(&context);
  for (const auto& parameter : node.parameters())
    ostream << Indent(context) << AsPrintable(&context, parameter);
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<ast::JsDocText>& printable) {
  const auto& node = *printable.node;
  return ostream << '|' << SourceCodeOf(node) << '|';
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<ast::JsDocType>& printable) {
  auto& context = *printable.context;
  const auto& node = *printable.node;
  return ostream << "#type " << AsPrintable(&context, node.type());
}

// Tokens
std::ostream& operator<<(std::ostream& ostream,
                         const Printable<ast::Comment>& printable) {
  const auto& node = *printable.node;
  return ostream << "#comment |" << SourceCodeOf(node) << '|';
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<ast::Empty>& printable) {
  return ostream << "#empty";
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<ast::JsDoc>& printable) {
  auto& context = *printable.context;
  const auto& node = *printable.node;
  ostream << "#jsdoc";
  IndentScope scope(&context);
  return ostream << AsPrintable(&context, node.document());
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<ast::Punctuator>& printable) {
  const auto& node = *printable.node;
  return ostream << SourceCodeOf(node);
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<ast::Name>& printable) {
  const auto& node = *printable.node;
  return ostream << SourceCodeOf(node);
}

// Types
std::ostream& operator<<(std::ostream& ostream,
                         const Printable<ast::AnyType>& printable) {
  return ostream << '*';
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<ast::FunctionType>& printable) {
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
                         const Printable<ast::InvalidType>& printable) {
  return ostream << "(invalid)";
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<ast::NullableType>& printable) {
  auto& context = *printable.context;
  const auto& node = *printable.node;
  return ostream << '?' << AsPrintable(&context, node.type());
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<ast::NonNullableType>& printable) {
  auto& context = *printable.context;
  const auto& node = *printable.node;
  return ostream << '!' << AsPrintable(&context, node.type());
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<ast::OptionalType>& printable) {
  auto& context = *printable.context;
  const auto& node = *printable.node;
  return ostream << '=' << AsPrintable(&context, node.type());
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<ast::RecordType>& printable) {
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
                         const Printable<ast::RestType>& printable) {
  auto& context = *printable.context;
  const auto& node = *printable.node;
  return ostream << "..." << AsPrintable(&context, node.type());
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<ast::TupleType>& printable) {
  auto& context = *printable.context;
  const auto& node = *printable.node;
  ostream << "tuple";
  IndentScope scope(&context);
  for (const auto& member : node.members())
    ostream << Indent(context) << AsPrintable(&context, member);
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<ast::TypeApplication>& printable) {
  auto& context = *printable.context;
  const auto& node = *printable.node;
  ostream << "application " << AsPrintable(&context, node.name());
  IndentScope scope(&context);
  for (const auto& parameter : node.parameters())
    ostream << Indent(context) << AsPrintable(&context, parameter);
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<ast::TypeGroup>& printable) {
  auto& context = *printable.context;
  const auto& node = *printable.node;
  ostream << "group";
  IndentScope scope(&context);
  return ostream << Indent(context) << AsPrintable(&context, node.type());
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<ast::TypeName>& printable) {
  auto& context = *printable.context;
  const auto& node = *printable.node;
  return ostream << AsPrintable(&context, node.name());
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<ast::UnionType>& printable) {
  auto& context = *printable.context;
  const auto& node = *printable.node;
  ostream << "union";
  IndentScope scope(&context);
  for (const auto& member : node.members())
    ostream << Indent(context) << AsPrintable(&context, member);
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<ast::UnknownType>& printable) {
  return ostream << '?';
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<ast::VoidType>& printable) {
  return ostream << "void";
}

// Dispatcher
std::ostream& operator<<(std::ostream& ostream,
                         const Printable<ast::Node>& printable) {
  auto& context = *printable.context;
  const auto& node = *printable.node;
#define V(name)             \
  if (node.Is<ast::name>()) \
    return ostream << Printable<ast::name>{&context, &node.As<ast::name>()};
  FOR_EACH_AST_BINDING_ELEMENT(V)
  FOR_EACH_AST_JSDOC(V)
  FOR_EACH_AST_TOKEN(V)
  FOR_EACH_AST_TYPE(V)
#undef V
  return ostream << SourceCodeOf(node);
}

}  // namespace

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableAstNode& printable) {
  Context context;
  return ostream << Printable<ast::Node>{&context, printable.node};
}

}  // namespace internal

internal::PrintableAstNode AsPrintableTree(const ast::Node& node) {
  return internal::PrintableAstNode{&node};
}

}  // namespace joana
