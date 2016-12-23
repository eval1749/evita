// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_AST_MODULE_H_
#define JOANA_AST_MODULE_H_

#include "joana/ast/container_node.h"

namespace joana {
namespace ast {

class StatementList;

//
// Module
//
class JOANA_AST_EXPORT Module final
    : public NodeTemplate<Node, const StatementList*> {
  DECLARE_CONCRETE_AST_NODE(Module, Node);

 public:
  ~Module() final;

  const StatementList& statements() const { return *member_at<0>(); }

 private:
  Module(const SourceCodeRange& range, const StatementList& statements);

  DISALLOW_COPY_AND_ASSIGN(Module);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_MODULE_H_
