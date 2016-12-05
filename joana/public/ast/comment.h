// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PUBLIC_AST_COMMENT_H_
#define JOANA_PUBLIC_AST_COMMENT_H_

#include "joana/public/ast/node.h"

#include "base/strings/string_piece.h"

namespace joana {
namespace ast {

class JOANA_PUBLIC_EXPORT Comment final : public Node {
  DECLARE_CONCRETE_AST_NODE(Comment, Node);

 public:
  ~Comment() final;

 private:
  explicit Comment(const SourceCodeRange& location);

  DISALLOW_COPY_AND_ASSIGN(Comment);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_PUBLIC_AST_COMMENT_H_
