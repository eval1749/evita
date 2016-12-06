// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PUBLIC_AST_INVALID_H_
#define JOANA_PUBLIC_AST_INVALID_H_

#include "joana/public/ast/node.h"
#include "joana/public/ast/node_forward.h"

namespace joana {
namespace ast {

class JOANA_PUBLIC_EXPORT Invalid final : public Node {
  DECLARE_CONCRETE_AST_NODE(Invalid, Node);

 public:
  ~Invalid() final;

  int error_code() const { return error_code_; }

 private:
  explicit Invalid(const SourceCodeRange& range, int error_code);

  // Implements |Node| members
  void PrintMoreTo(std::ostream* ostream) const final;

  const int error_code_;

  DISALLOW_COPY_AND_ASSIGN(Invalid);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_PUBLIC_AST_INVALID_H_
