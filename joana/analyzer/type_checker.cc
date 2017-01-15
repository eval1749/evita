// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stack>
#include <utility>

#include "joana/analyzer/type_checker.h"

#include "joana/analyzer/context.h"
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
  for (const auto& node : ast::NodeTraversal::DescendantsOf(toplevel_node))
    Visit(node);
}

// |ast::SyntaxVisitor| members
// Expressions
void TypeChecker::VisitInternal(const ast::ReferenceExpression& syntax,
                                const ast::Node& node) {
  const auto* value = context().TryValueOf(node);
  if (!value) {
    AddError(node, ErrorCode::TYPE_CHECKER_UNDEFINED_VARIABLE);
    return;
  }
  if (!value->Is<Variable>() || !value->As<Variable>().assignments().empty())
    return;
  AddError(node, ErrorCode::TYPE_CHECKER_UNINITIALIZED_VARIABLE);
}

// Types
void TypeChecker::VisitInternal(const ast::TypeName& syntax,
                                const ast::Node& node) {
  const auto* present = context().TryTypeOf(node);
  if (!present) {
    AddError(node, ErrorCode::TYPE_CHECKER_UNDEFINED_TYPE);
    return;
  }
}

}  // namespace analyzer
}  // namespace joana
