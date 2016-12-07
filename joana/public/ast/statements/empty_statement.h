// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PUBLIC_AST_STATEMENTS_EMPTY_STATEMENT_H_
#define JOANA_PUBLIC_AST_STATEMENTS_EMPTY_STATEMENT_H_

#include "joana/public/ast/statements/statement.h"

namespace joana {
namespace ast {

class Punctuator;

class JOANA_PUBLIC_EXPORT EmptyStatement : public Statement {
  DECLARE_CONCRETE_AST_NODE(EmptyStatement, Statement);

 public:
  ~EmptyStatement() override;

 protected:
  explicit EmptyStatement(const Punctuator& semi_colon);

 private:
  DISALLOW_COPY_AND_ASSIGN(EmptyStatement);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_PUBLIC_AST_STATEMENTS_EMPTY_STATEMENT_H_
