// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <ostream>
#include <type_traits>

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "joana/analyzer/type_forward.h"
#include "joana/analyzer/types.h"
#include "joana/analyzer/values.h"
#include "joana/ast/declarations.h"
#include "joana/ast/node.h"
#include "joana/ast/node_printer.h"
#include "joana/ast/tokens.h"

namespace joana {
namespace analyzer {

namespace {

//
// Printable
//
template <typename T>
struct Printable {
  static_assert(std::is_base_of<Type, T>::value, "Should be Type");
  const T* type;
};

Printable<Type> AsPrintable(const Type& type) {
  return Printable<Type>{&type};
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<Type>& printable);

// Type printer implementations
std::ostream& operator<<(std::ostream& ostream,
                         const Printable<AnyType>& printable) {
  return ostream << "*";
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<ClassType>& printable) {
  const auto& type = *printable.type;
  const auto& name = type.name();
  ostream << "class";
  if (name.Is<ast::Name>())
    ostream << ' ' << ast::AsSourceCode(name);
  const auto* delimiter = "<";
  for (const auto& parameter : type.value().parameters()) {
    ostream << delimiter << AsSourceCode(parameter.name());
    delimiter = ",";
  }
  if (*delimiter == ',')
    ostream << '>';
  return ostream << '@' << type.id();
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<FunctionType>& printable) {
  const auto& type = *printable.type;
  ostream << "function";
  if (type.has_parameters()) {
    auto delimiter = "<";
    for (const auto& parameter : type.parameters()) {
      ostream << delimiter << AsSourceCode(parameter.name());
      delimiter = ",";
    }
    ostream << '>';
  }
  ostream << '(';
  auto delimiter = "";
  if (type.kind() == FunctionTypeKind::Constructor) {
    ostream << "new:" << AsPrintable(type.this_type());
    delimiter = ",";
  } else if (!type.this_type().Is<VoidType>()) {
    ostream << "this:" << AsPrintable(type.this_type());
    delimiter = ",";
  }
  for (const auto& parameter_type : type.parameter_types()) {
    ostream << delimiter << AsPrintable(parameter_type);
    delimiter = ",";
  }
  ostream << ')';
  if (!type.return_type().Is<VoidType>())
    ostream << ':' << AsPrintable(type.return_type());
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<InvalidType>& printable) {
  return ostream << "%invalid%";
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<NilType>& printable) {
  return ostream << "%nil%";
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<NullType>& printable) {
  return ostream << "%null%";
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<PrimitiveType>& printable) {
  const auto& type = *printable.type;
  return ostream << "%" << ast::AsSourceCode(type.name()) << "%";
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<TupleType>& printable) {
  const auto& type = *printable.type;
  ostream << '[';
  auto* delimiter = "";
  for (const auto& member : type.members()) {
    ostream << delimiter << member;
    delimiter = ",";
  }
  return ostream << ']';
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<TypeApplication>& printable) {
  const auto& type = *printable.type;
  const auto& generic_type = type.generic_type();
  if (generic_type.Is<ClassType>())
    ostream << generic_type.As<ClassType>().name();
  else
    ostream << "function";
  ostream << '@' << generic_type.id() << '<';
  auto delimiter = "";
  for (const auto& argument : type.arguments()) {
    ostream << delimiter;
    delimiter = ",";
    const auto& parameter = argument.first;
    const auto& it = std::find_if(
        generic_type.parameters().begin(), generic_type.parameters().end(),
        [&](const auto& present) { return present == parameter; });
    if (it == generic_type.parameters().end()) {
      ostream << AsPrintable(*argument.second);
      continue;
    }
    ostream << *it;
  }
  return ostream << '>';
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<TypeName>& printable) {
  const auto& type = *printable.type;
  return ostream << ast::AsSourceCode(type.name()) << '@' << type.id();
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<TypeParameter>& printable) {
  const auto& type = *printable.type;
  return ostream << ast::AsSourceCode(type.name()) << '@' << type.id();
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<UnionType>& printable) {
  const auto& type = *printable.type;
  auto* delimiter = "";
  for (const auto& member : type.members()) {
    ostream << delimiter << member;
    delimiter = "|";
  }
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<UnspecifiedType>& printable) {
  return ostream << "?";
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<VoidType>& printable) {
  return ostream << "%void%";
}

// Dispatcher
std::ostream& operator<<(std::ostream& ostream,
                         const Printable<Type>& printable) {
  const auto& type = *printable.type;
#define V(name)                           \
  if (auto* derived = type.TryAs<name>()) \
    return ostream << Printable<name>{derived};
  FOR_EACH_ANALYZE_TYPE(V)
#undef V

  NOTREACHED() << "Need operator<< for " << type.class_name();
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

std::ostream& operator<<(std::ostream& ostream, std::set<const Type*>& types) {
  auto* delimiter = "";
  ostream << '{';
  for (const auto& type : types) {
    ostream << delimiter << type;
    delimiter = ",";
  }
  return ostream << '}';
}

std::ostream& operator<<(std::ostream& ostream,
                         std::vector<const Type*>& types) {
  auto* delimiter = "";
  ostream << '[';
  for (const auto& type : types) {
    ostream << delimiter << type;
    delimiter = ",";
  }
  return ostream << ']';
}

}  // namespace analyzer
}  // namespace joana
