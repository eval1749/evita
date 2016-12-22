// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_IR_NODE_FORWARD_H_
#define JOANA_IR_NODE_FORWARD_H_

#include <iosfwd>

#include "joana/ir/ir_export.h"

namespace joana {
namespace ir {

class Node;
class Operator;
class Type;

// Implemented in "joana/ir/node_printer.cc"
JOANA_IR_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                         const Node& node);

JOANA_IR_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                         const Node* node);

}  // namespace ir
}  // namespace joana

#endif  // JOANA_IR_NODE_FORWARD_H_
