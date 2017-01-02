// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <tuple>

#include "joana/ast/declarations.h"

namespace joana {
namespace ast {

//
// ArrowFunctionSyntax
//
ArrowFunctionSyntax::ArrowFunctionSyntax(FunctionKind kind)
    : SyntaxTemplate(
          kind,
          SyntaxCode::ArrowFunction,
          Format::Builder().set_arity(2).set_number_of_parameters(1).Build()) {}

ArrowFunctionSyntax::~ArrowFunctionSyntax() = default;

const Node& ArrowFunctionSyntax::BodyOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::ArrowFunction);
  return node.child_at(1);
}

const Node& ArrowFunctionSyntax::ParametersOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::ArrowFunction);
  return node.child_at(0);
}

//
// ClassSyntax
//
ClassSyntax::ClassSyntax()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::Class,
                     Format::Builder().set_arity(3).Build()) {}

ClassSyntax::~ClassSyntax() = default;

const Node& ClassSyntax::BodyOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::Class);
  return node.child_at(2);
}

const Node& ClassSyntax::HerisyntaxeOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::Class);
  return node.child_at(1);
}

const Node& ClassSyntax::NameOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::Class);
  return node.child_at(0);
}

//
// FunctionSyntax
//
FunctionSyntax::FunctionSyntax(FunctionKind kind)
    : SyntaxTemplate(
          kind,
          SyntaxCode::Function,
          Format::Builder().set_arity(3).set_number_of_parameters(1).Build()) {}

FunctionSyntax::~FunctionSyntax() = default;

const Node& FunctionSyntax::BodyOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::Function);
  return node.child_at(2);
}

const Node& FunctionSyntax::NameOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::Function);
  return node.child_at(0);
}

const Node& FunctionSyntax::ParametersOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::Function);
  return node.child_at(1);
}

//
// MethodSyntax
//
MethodSyntax::MethodSyntax(MethodKind method_kind, FunctionKind kind)
    : SyntaxTemplate(
          std::make_tuple(method_kind, kind),
          SyntaxCode::Method,
          Format::Builder().set_arity(3).set_number_of_parameters(2).Build()) {}

MethodSyntax::~MethodSyntax() = default;

const Node& MethodSyntax::BodyOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::Method);
  return node.child_at(2);
}

const Node& MethodSyntax::NameOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::Method);
  return node.child_at(0);
}

const Node& MethodSyntax::ParametersOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::Method);
  return node.child_at(1);
}

}  // namespace ast
}  // namespace joana
