// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/ast/syntax.h"

#include "joana/ast/node.h"

namespace joana {
namespace ast {

namespace {

enum FlagBit {
#define V(capital, underscore) kIndexOf##capital,
  FOR_EACH_AST_SYNTAX_FLAG_BIT(V)
#undef V
};

#define V(capital, underscore) \
  constexpr uint32_t k##capital = 1 << kIndexOf##capital;
FOR_EACH_AST_SYNTAX_FLAG_BIT(V)
#undef V

}  // namespace

// Syntax::Format::Format
Syntax::Format::Format() = default;
Syntax::Format::~Format() = default;

#define V(capital, underscore, ...)              \
  bool Syntax::Format::is_##underscore() const { \
    return (flags_ & k##capital) == k##capital;  \
  }
FOR_EACH_AST_SYNTAX_FLAG_BIT(V)
#undef V

// Syntax::Format::Builder
Syntax::Format::Builder::Builder() = default;
Syntax::Format::Builder::~Builder() = default;

Syntax::Format Syntax::Format::Builder::Build() {
  if (format_.is_literal())
    DCHECK_EQ(format_.arity(), 0);
  return format_;
}

Syntax::Format::Builder& Syntax::Format::Builder::set_arity(size_t value) {
  format_.arity_ = value;
  return *this;
}

Syntax::Format::Builder& Syntax::Format::Builder::set_number_of_parameters(
    size_t value) {
  format_.number_of_parameters_ = value;
  return *this;
}

#define V(capital, underscore, ...)                                      \
  Syntax::Format::Builder& Syntax::Format::Builder::set_is_##underscore( \
      bool value) {                                                      \
    if (value)                                                           \
      format_.flags_ |= k##capital;                                      \
    else                                                                 \
      format_.flags_ &= ~k##capital;                                     \
    return *this;                                                        \
  }
FOR_EACH_AST_SYNTAX_FLAG_BIT(V)
#undef V

//
// Syntax
//
Syntax::Syntax(SyntaxCode opcode, const Format& format)
    : format_(format), opcode_(opcode) {}

Syntax::~Syntax() = default;

bool Syntax::operator==(const Syntax& other) const {
  return this == &other;
}

bool Syntax::operator!=(const Syntax& other) const {
  return !operator==(other);
}

bool Syntax::operator==(SyntaxCode syntax_code) const {
  return opcode_ == syntax_code;
}

bool Syntax::operator!=(SyntaxCode syntax_code) const {
  return !operator==(syntax_code);
}

base::StringPiece Syntax::mnemonic() const {
  static const char* const kMnemonics[] = {"?0",
#define V(name) #name,
                                           FOR_EACH_AST_SYNTAX(V)
#undef V
  };

  const auto& it = std::begin(kMnemonics) + static_cast<size_t>(opcode_);
  DCHECK(it >= std::begin(kMnemonics));
  DCHECK(it < std::end(kMnemonics));
  return base::StringPiece(*it);
}

}  // namespace ast
}  // namespace joana
