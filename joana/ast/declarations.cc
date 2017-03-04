// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <tuple>

#include "joana/ast/declarations.h"

namespace joana {
namespace ast {

//
// Annotation
//
Annotation::Annotation()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::Annotation,
                     Format::Builder().set_arity(2).Build()) {}

Annotation::~Annotation() = default;

const Node& Annotation::AnnotatedOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::Annotation);
  return node.child_at(1);
}

const Node& Annotation::DocumentOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::Annotation);
  return node.child_at(0);
}

//
// ArrowFunction
//
ArrowFunction::ArrowFunction(FunctionKind kind)
    : SyntaxTemplate(
          kind,
          SyntaxCode::ArrowFunction,
          Format::Builder().set_arity(2).set_number_of_parameters(1).Build()) {}

ArrowFunction::~ArrowFunction() = default;

const Node& ArrowFunction::BodyOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::ArrowFunction);
  return node.child_at(1);
}

const Node& ArrowFunction::ParametersOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::ArrowFunction);
  return node.child_at(0);
}

//
// Class
//
Class::Class()
    : SyntaxTemplate(std::tuple<>(),
                     SyntaxCode::Class,
                     Format::Builder().set_arity(3).Build()) {}

Class::~Class() = default;

const Node& Class::BodyOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::Class);
  return node.child_at(2);
}

const Node& Class::HeritageOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::Class);
  return node.child_at(1);
}

const Node& Class::NameOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::Class);
  return node.child_at(0);
}

//
// Declaration
//
Declaration::Declaration()
    : Syntax(SyntaxCode::Declaration, Format::Builder().set_arity(2).Build()) {}

Declaration::~Declaration() = default;

const Node& Declaration::ExpressionOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::Declaration);
  return node.child_at(0);
}

const Node& Declaration::InitializerOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::Declaration);
  return node.child_at(1);
}

//
// Function
//
Function::Function(FunctionKind kind)
    : SyntaxTemplate(
          kind,
          SyntaxCode::Function,
          Format::Builder().set_arity(3).set_number_of_parameters(1).Build()) {}

Function::~Function() = default;

const Node& Function::BodyOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::Function);
  return node.child_at(2);
}

const Node& Function::NameOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::Function);
  return node.child_at(0);
}

const Node& Function::ParametersOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::Function);
  return node.child_at(1);
}

//
// Method
//
Method::Method(MethodKind method_kind, FunctionKind kind)
    : SyntaxTemplate(
          std::make_tuple(method_kind, kind),
          SyntaxCode::Method,
          Format::Builder().set_arity(3).set_number_of_parameters(2).Build()) {}

Method::~Method() = default;

const Node& Method::BodyOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::Method);
  return node.child_at(2);
}

FunctionKind Method::FunctionKindOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::Method);
  return node.syntax().As<Method>().kind();
}

bool Method::IsStatic(const ast::Node& node) {
  return MethodKindOf(node) == MethodKind::Static;
}

MethodKind Method::MethodKindOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::Method);
  return node.syntax().As<Method>().method_kind();
}

const Node& Method::NameOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::Method);
  return node.child_at(0);
}

const Node& Method::ParametersOf(const Node& node) {
  DCHECK_EQ(node, SyntaxCode::Method);
  return node.child_at(1);
}

}  // namespace ast
}  // namespace joana
