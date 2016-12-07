// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PUBLIC_AST_NAME_H_
#define JOANA_PUBLIC_AST_NAME_H_

#include "joana/public/ast/node.h"

#include "base/strings/string_piece.h"

namespace joana {
namespace ast {

class JOANA_PUBLIC_EXPORT Name final : public Node {
  DECLARE_CONCRETE_AST_NODE(Name, Node);

 public:
  ~Name() final;

  int number() const { return number_; }

 private:
  Name(const SourceCodeRange& range, int number);

  // Implements |Node| members
  void PrintMoreTo(std::ostream* ostream) const final;

  const int number_;

  DISALLOW_COPY_AND_ASSIGN(Name);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_PUBLIC_AST_NAME_H_
