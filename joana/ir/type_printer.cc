// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "joana/ir/composite_types.h"
#include "joana/ir/primitive_types.h"

namespace joana {
namespace ir {

namespace {

//
// PrintableType
//
struct PrintableType {
  const Type* type;
};

std::ostream& operator<<(std::ostream& ostream, const PrintableType& printable);

PrintableType AsPrintable(const Type& type) {
  return PrintableType{&type};
}

//
// PrintableXXX
//
#define V(capital, ...)                                       \
  struct Printable##capital {                                 \
    const capital##Type* type;                                \
  };                                                          \
                                                              \
  Printable##capital AsPrintable(const capital##Type& type) { \
    return Printable##capital{&type};                         \
  }
FOR_EACH_IR_TYPE(V)
#undef V

//
// PrimitiveType printers
//
#define V(name, ...)                                           \
  std::ostream& operator<<(std::ostream& ostream,              \
                           const Printable##name& printable) { \
    return ostream << #name;                                   \
  }
FOR_EACH_IR_PRIMITIVE_TYPE(V)
#undef V

//
// CompositeType printers
//
#define IMPLEMENT_IR_COMPOSITE_TYPE_1(name, member)            \
  std::ostream& operator<<(std::ostream& ostream,              \
                           const Printable##name& printable) { \
    const auto& type = *printable.type;                        \
    return ostream << #name << '<' << type.member() << '>';    \
  }

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableFunction& printable) {
  const auto& type = *printable.type;
  ostream << "Function<(";
  auto* delimiter = "";
  for (const auto& parameter_type : type.parameters_type().members()) {
    ostream << delimiter << AsPrintable(parameter_type);
    delimiter = ", ";
  }
  return ostream << ") => " << AsPrintable(type.return_type()) << '>';
}

IMPLEMENT_IR_COMPOSITE_TYPE_1(Reference, to);

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableTuple& printable) {
  const auto& type = *printable.type;
  ostream << "Tuple<";
  auto* delimiter = "";
  for (const auto& member : type.members()) {
    ostream << delimiter << AsPrintable(member);
    delimiter = ", ";
  }
  return ostream << '>';
}

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableUnion& printable) {
  const auto& type = *printable.type;
  ostream << "Union<";
  auto* delimiter = "";
  for (const auto& member : type.members()) {
    ostream << delimiter << AsPrintable(member);
    delimiter = ", ";
  }
  return ostream << '>';
}

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableType& printable) {
  const auto& type = *printable.type;
#define V(capital, ...)         \
  if (type.Is<capital##Type>()) \
    return ostream << AsPrintable(type.As<capital##Type>());
  FOR_EACH_IR_TYPE(V)
#undef V
  NOTREACHED() << "No printer?";
  return ostream;
}

}  // namespace

std::ostream& operator<<(std::ostream& ostream, const Type& type) {
  return ostream << AsPrintable(type);
}

std::ostream& operator<<(std::ostream& ostream, const Type* type) {
  if (!type)
    return ostream << "(null)";
  return ostream << *type;
}

}  // namespace ir
}  // namespace joana
