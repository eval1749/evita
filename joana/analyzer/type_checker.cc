// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stack>
#include <utility>

#include "joana/analyzer/type_checker.h"

#include "joana/analyzer/context.h"
#include "joana/analyzer/environment.h"
#include "joana/analyzer/error_codes.h"
#include "joana/analyzer/factory.h"
#include "joana/analyzer/types.h"
#include "joana/analyzer/values.h"
#include "joana/ast/expressions.h"
#include "joana/ast/node.h"
#include "joana/ast/node_traversal.h"
#include "joana/ast/types.h"

namespace joana {
namespace analyzer {

//
// TypeChecker
//
TypeChecker::TypeChecker(Context* context) : Pass(context) {}
TypeChecker::~TypeChecker() = default;

// The entry point
void TypeChecker::RunOn(const ast::Node& toplevel_node) {
  environment_ = &context().EnvironmentOf(toplevel_node);
  for (const auto& node : ast::NodeTraversal::DescendantsOf(toplevel_node))
    Visit(node);
}

const Type* TypeChecker::FindType(const ast::Node& name) const {
  DCHECK_EQ(name, ast::SyntaxCode::Name);
  for (const auto* runner = environment_; runner; runner = runner->outer()) {
    if (const auto* present = runner->FindType(name))
      return present;
  }
  return nullptr;
}

const Variable* TypeChecker::FindVariable(const ast::Node& name) const {
  DCHECK_EQ(name, ast::SyntaxCode::Name);
  for (const auto* runner = environment_; runner; runner = runner->outer()) {
    if (const auto* present = runner->FindVariable(name))
      return present;
  }
  return nullptr;
}

// |ast::SyntaxVisitor| members
// Expressions
void TypeChecker::VisitInternal(const ast::ReferenceExpression& syntax,
                                const ast::Node& node) {
  const auto* value = context().TryValueOf(node);
  if (!value) {
    AddError(node, ErrorCode::TYPE_CHECKER_UNDEFIEND_VARIABLE);
    return;
  }
  if (!value->Is<Variable>() || !value->As<Variable>().assignments().empty())
    return;
  const auto* variable = FindVariable(ast::ReferenceExpression::NameOf(node));
  if (variable && !variable->assignments().empty())
    return;
  AddError(node, ErrorCode::TYPE_CHECKER_UNDEFIEND_VARIABLE);
}

// Types
void TypeChecker::VisitInternal(const ast::TypeName& syntax,
                                const ast::Node& node) {
  const auto* present = context().TryTypeOf(node);
  if (!present && !FindType(ast::TypeName::NameOf(node))) {
    AddError(node, ErrorCode::TYPE_CHECKER_UNDEFIEND_TYPE);
    return;
  }
}

}  // namespace analyzer
}  // namespace joana
