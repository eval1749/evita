// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_PRINT_AS_TREE_H_
#define JOANA_ANALYZER_PRINT_AS_TREE_H_

#include <iosfwd>

namespace joana {

namespace ast {
class Node;
}

namespace analyzer {

class Context;

struct PrintableTreeNode {
  const Context* context;
  const ast::Node* node;
};

PrintableTreeNode AsPrintableTree(const Context& context,
                                  const ast::Node& node);

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableTreeNode& printable);

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_PRINT_AS_TREE_H_
