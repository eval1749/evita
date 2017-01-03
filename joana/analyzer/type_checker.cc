// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stack>
#include <utility>

#include "joana/analyzer/type_checker.h"

#include "joana/analyzer/environment.h"
#include "joana/analyzer/error_codes.h"
#include "joana/analyzer/factory.h"
#include "joana/ast/expressions.h"
#include "joana/ast/node.h"

namespace joana {
namespace analyzer {

//
// TypeChecker
//
TypeChecker::TypeChecker(Context* context) : Pass(context) {}
TypeChecker::~TypeChecker() = default;

void TypeChecker::RunOn(const ast::Node& node) {
  environment_ = &factory().EnvironmentOf(node);
  ast::DepthFirstTraverse(this, node);
}

Value* TypeChecker::TryValueOf(const ast::Node& node) const {
  for (const auto* runner = environment_; runner; runner = runner->outer()) {
    if (auto* present = runner->TryValueOf(node))
      return present;
  }
  return nullptr;
}

// |ast::SyntaxVisitor| members
void TypeChecker::Visit(const ast::ReferenceExpression& syntax,
                        const ast::Node& node) {
  if (factory().TryValueOf(node))
    return;
  AddError(node, ErrorCode::TYPE_CHECKER_UNDEFIEND_VARIABLE);
}

}  // namespace analyzer
}  // namespace joana
