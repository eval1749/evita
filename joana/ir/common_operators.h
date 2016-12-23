// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_IR_COMMON_OPERATORS_H_
#define JOANA_IR_COMMON_OPERATORS_H_

#include "joana/ir/operator.h"

namespace joana {
namespace ir {

DECLARE_IR_OPERATOR_0(Exit)
DECLARE_IR_OPERATOR_0(If)
DECLARE_IR_OPERATOR_0(IfException)
DECLARE_IR_OPERATOR_0(IfFalse)
DECLARE_IR_OPERATOR_0(IfSuccess)
DECLARE_IR_OPERATOR_0(IfTrue)
DECLARE_IR_OPERATOR_1(LiteralBool, bool, data)
DECLARE_IR_OPERATOR_1(LiteralFloat64, float64_t, data)
DECLARE_IR_OPERATOR_1(LiteralInt64, int64_t, data)
DECLARE_IR_OPERATOR_1(LiteralString, base::StringPiece16, data)
DECLARE_IR_OPERATOR_0(LiteralVoid)
DECLARE_IR_OPERATOR_1(Projection, size_t, index)
DECLARE_IR_OPERATOR_0(Ret)
DECLARE_IR_OPERATOR_0(Start)
DECLARE_IR_OPERATOR_1(Tuple, size_t, size)

}  // namespace ir
}  // namespace joana

#endif  // JOANA_IR_COMMON_OPERATORS_H_
