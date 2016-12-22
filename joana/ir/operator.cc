// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>

#include "joana/ir/operator.h"

namespace joana {
namespace ir {

// Operator::Format::Format
Operator::Format::Format() = default;
Operator::Format::~Format() = default;

// Operator::Format::Builder
Operator::Format::Builder::Builder() = default;
Operator::Format::Builder::~Builder() = default;

Operator::Format::Builder& Operator::Format::Builder::set_number_of_members(
    size_t value) {
  format_.number_of_members_ = value;
  return *this;
}

//
// Operator
//
Operator::Operator(OperationCode opcode, const Format& format)
    : format_(format), opcode_(opcode) {}

Operator::~Operator() = default;

bool Operator::operator==(const Operator& other) const {
  return this == &other;
}

bool Operator::operator!=(const Operator& other) const {
  return !operator==(other);
}

base::StringPiece Operator::mnemonic() const {
  static const char* const kMnemonics[] = {
#define V(name) #name,
      FOR_EACH_IR_OPERATOR(V)
#undef V
  };

  const auto& it = std::begin(kMnemonics) + static_cast<size_t>(opcode_);
  DCHECK(it >= std::begin(kMnemonics));
  DCHECK(it < std::end(kMnemonics));
  return base::StringPiece(*it);
}

size_t Operator::ComputeHashCode() const {
  return std::hash<OperationCode>()(opcode_);
}

}  // namespace ir
}  // namespace joana
