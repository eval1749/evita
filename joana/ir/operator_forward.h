// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_IR_OPERATOR_FORWARD_H_
#define JOANA_IR_OPERATOR_FORWARD_H_

#include <iosfwd>

#include "joana/base/float_type.h"
#include "joana/ir/ir_export.h"
#include "joana/ir/type_forward.h"

namespace joana {
namespace ir {

#define FOR_EACH_COMMON_IR_OPERATOR(V) \
  V(Exit)                              \
  V(If)                                \
  V(IfException)                       \
  V(IfFalse)                           \
  V(IfSuccess)                         \
  V(IfTrue)                            \
  V(LiteralBool)                       \
  V(LiteralFloat64)                    \
  V(LiteralInt64)                      \
  V(LiteralString)                     \
  V(LiteralVoid)                       \
  V(Projection)                        \
  V(Ret)                               \
  V(Start)                             \
  V(Tuple)

#define FOR_EACH_IR_OPERATOR(V) FOR_EACH_COMMON_IR_OPERATOR(V)

class Operator;
enum class OperationCode;
#define V(name) class name##Operator;
FOR_EACH_IR_OPERATOR(V)
#undef V

// Implemented in "joana/ir/operator_printer.cc"
JOANA_IR_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                         const Operator& op);

JOANA_IR_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                         const Operator* op);

}  // namespace ir
}  // namespace joana

#endif  // JOANA_IR_OPERATOR_FORWARD_H_
