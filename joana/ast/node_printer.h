// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_AST_NODE_PRINTER_H_
#define JOANA_AST_NODE_PRINTER_H_

#include <iosfwd>

#include "joana/ast/ast_export.h"

namespace joana {
namespace ast {

class Node;

struct NodeAsSourceCode {
  const Node* node;
};

JOANA_AST_EXPORT NodeAsSourceCode AsSourceCode(const Node& node);

JOANA_AST_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          const NodeAsSourceCode& printable);

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_NODE_PRINTER_H_
