// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "joana/base/escaped_string_piece.h"
#include "joana/ir/common_operators.h"

namespace joana {
namespace ir {

namespace {

//
// PrintableOperator
//
struct PrintableOperator {
  const Operator* op;
};

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableOperator& printable);

PrintableOperator AsPrintable(const Operator& op) {
  return PrintableOperator{&op};
}

//
// PrintableXXX
//
#define V(capital, ...)                                         \
  struct Printable##capital {                                   \
    const capital##Operator* op;                                \
  };                                                            \
                                                                \
  Printable##capital AsPrintable(const capital##Operator& op) { \
    return Printable##capital{&op};                             \
  }
FOR_EACH_IR_OPERATOR(V)
#undef V

#define IMPLEMENT_IR_OPERATOR_PRINTER_0(capital)                  \
  std::ostream& operator<<(std::ostream& ostream,                 \
                           const Printable##capital& printable) { \
    const auto& op = *printable.op;                               \
    return ostream << op.mnemonic();                              \
  }

#define IMPLEMENT_IR_OPERATOR_PRINTER_1(capital, member)          \
  std::ostream& operator<<(std::ostream& ostream,                 \
                           const Printable##capital& printable) { \
    const auto& op = *printable.op;                               \
    return ostream << op.mnemonic() << '<' << op.member() << '>'; \
  }

IMPLEMENT_IR_OPERATOR_PRINTER_0(Exit)
IMPLEMENT_IR_OPERATOR_PRINTER_1(LiteralBool, data)
IMPLEMENT_IR_OPERATOR_PRINTER_1(LiteralFloat64, data)
IMPLEMENT_IR_OPERATOR_PRINTER_1(LiteralInt64, data)

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableLiteralString& printable) {
  const auto& op = *printable.op;
  return ostream << op.mnemonic() << '<' << EscapedStringPiece16(op.data(), '"')
                 << '>';
}

IMPLEMENT_IR_OPERATOR_PRINTER_0(LiteralVoid)
IMPLEMENT_IR_OPERATOR_PRINTER_1(Projection, index)
IMPLEMENT_IR_OPERATOR_PRINTER_0(Ret)
IMPLEMENT_IR_OPERATOR_PRINTER_0(Start)
IMPLEMENT_IR_OPERATOR_PRINTER_1(Tuple, size)

// |PrintableOperator| is a dispatcher.
std::ostream& operator<<(std::ostream& ostream,
                         const PrintableOperator& printable) {
  const auto& op = *printable.op;
#define V(capital, ...)           \
  if (op.Is<capital##Operator>()) \
    return ostream << Printable##capital{&op.As<capital##Operator>()};
  FOR_EACH_IR_OPERATOR(V)
#undef V
  NOTREACHED() << "No printer?";
  return ostream;
}

}  // namespace

std::ostream& operator<<(std::ostream& ostream, const Operator& op) {
  return ostream << AsPrintable(op);
}

std::ostream& operator<<(std::ostream& ostream, const Operator* op) {
  if (!op)
    return ostream << "(null)";
  return ostream << *op;
}

}  // namespace ir
}  // namespace joana
