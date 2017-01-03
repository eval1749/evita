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

namespace {

void DepthFirstTraverse(ast::SyntaxVisitor* visitor,
                        const ast::Node& start_node) {
  std::stack<std::pair<const ast::Node*, size_t>> stack;
  stack.push(std::make_pair(&start_node, 0));
  start_node.syntax().Accept(visitor, start_node);
  while (!stack.empty()) {
    const auto& container = *stack.top().first;
    const auto index = stack.top().second;
    if (index == container.arity()) {
      stack.pop();
      continue;
    }
    stack.top().second = index + 1;
    const auto& child = container.child_at(index);
    stack.push(std::make_pair(&child, 0));
    child.syntax().Accept(visitor, child);
  }
}

}  // namespace

//
// TypeChecker
//
TypeChecker::TypeChecker(Context* context) : Pass(context) {}
TypeChecker::~TypeChecker() = default;

void TypeChecker::RunOn(const ast::Node& node) {
  environment_ = &factory().EnvironmentOf(node);
  DepthFirstTraverse(this, node);
}

Value* TypeChecker::TryValueOf(const ast::Node& node) const {
  for (const auto* runner = environment_; runner; runner = runner->outer()) {
    if (auto* present = runner->TryValueOf(node))
      return present;
  }
  return nullptr;
}

// |ast::SyntaxVisitor| members
void TypeChecker::Visit(const ast::ReferenceExpressionSyntax& syntax,
                        const ast::Node& node) {
  if (factory().TryValueOf(node))
    return;
  AddError(node, ErrorCode::TYPE_CHECKER_UNDEFIEND_VARIABLE);
}

}  // namespace analyzer
}  // namespace joana
