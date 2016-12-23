// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "joana/base/escaped_string_piece.h"
#include "joana/ir/common_operators.h"
#include "joana/ir/composite_types.h"
#include "joana/ir/node.h"
#include "joana/ir/operator_forward.h"
#include "joana/ir/primitive_types.h"
#include "joana/ir/type_forward.h"

namespace joana {
namespace ir {

namespace {

//
// PrintableNode
//
struct PrintableNode {
  const Node* node;
};

PrintableNode AsPrintable(const Node& node) {
  return PrintableNode{&node};
}

//
// PrintableOperand
//
struct PrintableOperand {
  const Node* node;
};

PrintableOperand AsPrintableOperand(const Node& node) {
  return PrintableOperand{&node};
}

char TypeCharOf(const Type& type) {
  if (type.Is<BoolType>())
    return 'b';
  if (type.Is<ControlType>())
    return 'c';
  if (type.Is<EffectType>())
    return 'e';
  if (type.Is<Float64Type>())
    return 'f';
  if (type.Is<Int64Type>())
    return 'i';
  if (type.Is<TupleType>())
    return 't';
  return 'r';
}

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableOperand& printable) {
  auto& node = *printable.node;
  auto& op = node.op();
  if (auto* literal = op.TryAs<LiteralBoolOperator>())
    return ostream << std::boolalpha << literal->data();
  if (auto* literal = op.TryAs<LiteralFloat64Operator>())
    return ostream << literal->data();
  if (auto* literal = op.TryAs<LiteralInt64Operator>())
    return ostream << literal->data();
  if (auto* literal = op.TryAs<LiteralStringOperator>())
    return ostream << EscapedStringPiece16(literal->data(), '"');
  if (op.Is<LiteralVoidOperator>())
    return ostream << "void";
  return ostream << '%' << TypeCharOf(node.output_type()) << node.id();
}

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableNode& printable) {
  auto& node = *printable.node;
  ostream << node.id() << ':' << node.op() << '(';
  const char* delimiter = "";
  for (const auto& input : node.inputs()) {
    ostream << delimiter << AsPrintableOperand(input);
    delimiter = ", ";
  }
  return ostream << "):" << node.output_type();
}

}  // namespace

std::ostream& operator<<(std::ostream& ostream, const Node& node) {
  return ostream << AsPrintable(node);
}

std::ostream& operator<<(std::ostream& ostream, const Node* node) {
  if (!node)
    return ostream << "(null)";
  return ostream << *node;
}

}  // namespace ir
}  // namespace joana
