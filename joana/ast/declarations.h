// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_AST_DECLARATIONS_H_
#define JOANA_AST_DECLARATIONS_H_

#include "joana/ast/node.h"
#include "joana/ast/node_forward.h"

namespace joana {
namespace ast {

class ExpressionList;
class Statement;

//
// FunctionKind
//
enum class FunctionKind {
  Invalid,
  Async,
  Generator,
  Getter,
  Normal,
  Setter,
};

//
// MethodKind
//
enum class MethodKind {
  NonStatic,
  Static,
};

//
// Declaration is a base class of declaration nodes.
//
class JOANA_AST_EXPORT Declaration : public Node {
  DECLARE_ABSTRACT_AST_NODE(Declaration, Node);

 public:
  ~Declaration() override;

 protected:
  explicit Declaration(const SourceCodeRange& range);

 private:
  DISALLOW_COPY_AND_ASSIGN(Declaration);
};

//
// ArrowFunction
//
// The parameter list is represented by |ast::Expression| with:
//  - () = EmptyExpression
//  - x = ReferenceExpression
//  - (a, b, ...) = GroupExpression with CommaExpression
//
class JOANA_AST_EXPORT ArrowFunction final
    : public NodeTemplate<Declaration,
                          FunctionKind,
                          const Expression*,
                          const ArrowFunctionBody*> {
  DECLARE_CONCRETE_AST_NODE(ArrowFunction, Declaration);

 public:
  ~ArrowFunction() final;

  const ArrowFunctionBody& body() const { return *member_at<2>(); }
  FunctionKind kind() const { return member_at<0>(); }
  const Expression& parameter_list() const { return *member_at<1>(); }

 private:
  // |statement| should be either expression statement or block statement.
  ArrowFunction(const SourceCodeRange& range,
                FunctionKind kind,
                const Expression& parameter_list,
                const ArrowFunctionBody& body);

  DISALLOW_COPY_AND_ASSIGN(ArrowFunction);
};

//
// Class
//
class JOANA_AST_EXPORT Class final
    : public NodeTemplate<Declaration, Token*, Expression*, Expression*> {
  DECLARE_CONCRETE_AST_NODE(Class, Declaration);

 public:
  ~Class() final;

  const Expression& body() const { return *member_at<2>(); }
  const Token& name() const { return *member_at<0>(); }
  const Expression& heritage() const { return *member_at<1>(); }

 private:
  Class(const SourceCodeRange& range,
        Token* name,
        Expression* heritage,
        Expression* body);

  DISALLOW_COPY_AND_ASSIGN(Class);
};

//
// Function
//
// The parameter list is represented by |ast::Expression| with:
//  - () = EmptyExpression
//  - (a, b, ...) = GroupExpression with CommaExpression
//
class JOANA_AST_EXPORT Function final : public NodeTemplate<Declaration,
                                                            FunctionKind,
                                                            Token*,
                                                            Expression*,
                                                            Statement*> {
  DECLARE_CONCRETE_AST_NODE(Function, Declaration);

 public:
  ~Function() final;

  const Statement& body() const { return *member_at<3>(); }
  FunctionKind kind() const { return member_at<0>(); }
  const Token& name() const { return *member_at<1>(); }
  const Expression& parameter_list() const { return *member_at<2>(); }

 private:
  // |statement| should be either expression statement or block statement.
  Function(const SourceCodeRange& range,
           FunctionKind kind,
           Token* name,
           Expression* parameter_list,
           Statement* body);

  DISALLOW_COPY_AND_ASSIGN(Function);
};

//
// Method
//
// The parameter list is represented by |ast::Expression| with:
//  - () = EmptyExpression
//  - (a, b, ...) = GroupExpression with CommaExpression
//
class JOANA_AST_EXPORT Method final : public NodeTemplate<Declaration,
                                                          MethodKind,
                                                          FunctionKind,
                                                          Expression*,
                                                          Expression*,
                                                          Statement*> {
  DECLARE_CONCRETE_AST_NODE(Method, Declaration);

 public:
  ~Method() final;

  const Statement& body() const { return *member_at<4>(); }
  bool is_static() const { return member_at<0>() == MethodKind::Static; }
  FunctionKind kind() const { return member_at<1>(); }
  const Expression& name() const { return *member_at<2>(); }
  const Expression& parameter_list() const { return *member_at<3>(); }

 private:
  // |statement| should be either expression statement or block statement.
  Method(const SourceCodeRange& range,
         MethodKind is_static,
         FunctionKind kind,
         Expression* name,
         Expression* parameter_list,
         Statement* body);

  DISALLOW_COPY_AND_ASSIGN(Method);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_DECLARATIONS_H_
