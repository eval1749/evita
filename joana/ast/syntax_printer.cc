// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "joana/ast/syntax.h"

#include "joana/ast/types.h"

namespace joana {
namespace ast {

namespace {

template <typename T>
struct Printable {
  const T* value;
};

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<FunctionTypeSyntax>& printable) {
  const auto& syntax = *printable.value;
  switch (syntax.kind()) {
    case FunctionTypeKind::Normal:
      return ostream << "<Normal>";
    case FunctionTypeKind::This:
      return ostream << "<This>";
    case FunctionTypeKind::New:
      return ostream << "<New>";
  }
  NOTREACHED() << "Invalid FunctionTypeKind="
               << static_cast<int>(syntax.kind());
  return ostream;
}

}  // namespace

std::ostream& operator<<(std::ostream& ostream, const Syntax& syntax) {
  ostream << syntax.mnemonic();
  if (syntax.format().number_of_parameters() == 0)
    return ostream;
#define PRINT_PARAMETERS_FOR(name) \
  if (syntax.Is<name##Syntax>())   \
    return ostream << Printable<name##Syntax>{&syntax.As<name##Syntax>()};

  PRINT_PARAMETERS_FOR(FunctionType);
#undef PRINT_PARAMETERS
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream, const Syntax* syntax) {
  if (!syntax)
    return ostream << "(null)";
  return ostream << *syntax;
}

}  // namespace ast
}  // namespace joana
