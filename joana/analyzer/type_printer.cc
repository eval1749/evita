// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>
#include <type_traits>

#include "base/logging.h"
#include "joana/analyzer/type_forward.h"
#include "joana/analyzer/types.h"
#include "joana/ast/node.h"

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
                         const Printable<GenericType>& printable) {
  const auto& type = *printable.type;
  return ostream << "$GenericType@" << type.id() << ' ' << type.node();
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<PrimitiveType>& printable) {
  const auto& type = *printable.type;
  return ostream << "$PrimitiveType@" << type.id() << ' ' << type.node();
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<TypeApplication>& printable) {
  const auto& type = *printable.type;
  return ostream << "$TypeApplication@" << type.id() << ' ' << type.node();
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<TypeName>& printable) {
  const auto& type = *printable.type;
  return ostream << "$TypeName@" << type.id() << ' ' << type.node();
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<TypeParameter>& printable) {
  const auto& type = *printable.type;
  return ostream << "$TypeParameter@" << type.id() << ' ' << type.node();
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<TypeReference>& printable) {
  const auto& type = *printable.type;
  return ostream << "$TypeReference@" << type.id() << ' ' << type.node();
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

}  // namespace analyzer
}  // namespace joana
