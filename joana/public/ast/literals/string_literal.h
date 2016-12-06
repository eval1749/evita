// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PUBLIC_AST_LITERALS_STRING_LITERAL_H_
#define JOANA_PUBLIC_AST_LITERALS_STRING_LITERAL_H_

#include "joana/public/ast/literals/literal.h"

#include "base/strings/string16.h"
#include "base/strings/string_piece.h"

namespace joana {
namespace ast {

class JOANA_PUBLIC_EXPORT StringLiteral final : public Literal {
  DECLARE_CONCRETE_AST_NODE(StringLiteral, Literal);

 public:
  ~StringLiteral() final;

  const base::string16& data() const { return data_; }

 private:
  StringLiteral(const SourceCodeRange& range, base::StringPiece16 data);

  // Implements |Node| members
  void PrintMoreTo(std::ostream* ostream) const final;

  const base::string16 data_;

  DISALLOW_COPY_AND_ASSIGN(StringLiteral);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_PUBLIC_AST_LITERALS_STRING_LITERAL_H_
