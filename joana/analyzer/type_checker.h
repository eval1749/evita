// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_TYPE_CHECKER_H_
#define JOANA_ANALYZER_TYPE_CHECKER_H_

#include "joana/analyzer/pass.h"

#include "joana/ast/syntax_visitor.h"

namespace joana {

namespace ast {
class ReferenceExpressionSyntax;
}

namespace analyzer {

class Environment;
class Variable;

//
// TypeChecker
//
class TypeChecker final : public Pass, public ast::SyntaxVisitor {
 public:
  explicit TypeChecker(Context* context);
  ~TypeChecker();

  void RunOn(const ast::Node& node);

 private:
  Variable* TryValueOf(const ast::Node& node) const;

  // |ast::SyntaxVisitor| members
  // Expressions
  void Visit(const ast::ReferenceExpression& syntax,
             const ast::Node& node) final;

  // Types
  void Visit(const ast::TypeName& syntax, const ast::Node& node) final;

  Environment* environment_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(TypeChecker);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_TYPE_CHECKER_H_
