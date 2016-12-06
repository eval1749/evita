// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PUBLIC_AST_PUNCTUATOR_H_
#define JOANA_PUBLIC_AST_PUNCTUATOR_H_

#include "joana/public/ast/lexical_grammar.h"
#include "joana/public/ast/node.h"

namespace joana {
namespace ast {

enum class PunctuatorKind {
#define V(text, capital, upper) capital,
  FOR_EACH_JAVASCRIPT_PUNCTUATOR(V)
#undef V
};

class JOANA_PUBLIC_EXPORT Punctuator final : public Node {
  DECLARE_CONCRETE_AST_NODE(Punctuator, Node);

 public:
  explicit Punctuator(const SourceCodeRange& range, PunctuatorKind kind);
  ~Punctuator() final;

  PunctuatorKind kind() const { return kind_; }

 private:
  const PunctuatorKind kind_;

  DISALLOW_COPY_AND_ASSIGN(Punctuator);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_PUBLIC_AST_PUNCTUATOR_H_
