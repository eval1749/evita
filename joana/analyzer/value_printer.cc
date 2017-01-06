// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "base/logging.h"
#include "joana/analyzer/value_forward.h"
#include "joana/analyzer/values.h"
#include "joana/ast/node.h"

namespace joana {
namespace analyzer {

namespace {

//
// Printable
//
template <typename T>
struct Printable {
  static_assert(std::is_base_of<Value, T>::value, "Should be Value");
  const T* value;
};

Printable<Value> AsPrintable(const Value& value) {
  return Printable<Value>{&value};
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<Class>& printable) {
  const auto& value = *printable.value;
  return ostream << "$Class@" << value.id() << ' ' << value.node();
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<Function>& printable) {
  const auto& value = *printable.value;
  return ostream << "$Function@" << value.id() << ' ' << value.node();
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<Method>& printable) {
  const auto& value = *printable.value;
  return ostream << "$Method@" << value.id() << ' ' << value.node();
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<OrdinaryObject>& printable) {
  const auto& value = *printable.value;
  return ostream << "$OrdinaryObject@" << value.id() << ' ' << value.node();
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<Property>& printable) {
  const auto& value = *printable.value;
  return ostream << "$Property@" << value.id() << ' ' << value.node();
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<Undefined>& printable) {
  const auto& value = *printable.value;
  return ostream << "$Undefined@" << value.id() << ' ' << value.node();
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<Variable>& printable) {
  const auto& value = *printable.value;
  return ostream << "$Variable@" << value.id() << ' ' << value.node();
}

// Dispatcher
std::ostream& operator<<(std::ostream& ostream,
                         const Printable<Value>& printable) {
  const auto& value = *printable.value;
#define V(name)                            \
  if (auto* derived = value.TryAs<name>()) \
    return ostream << Printable<name>{derived};
  FOR_EACH_ANALYZE_VALUE(V)
#undef V

  NOTREACHED() << "Need operator<< for " << value.class_name();
  return ostream;
}

}  // namespace

std::ostream& operator<<(std::ostream& ostream, const Value& value) {
  return ostream << AsPrintable(value);
}

std::ostream& operator<<(std::ostream& ostream, const Value* value) {
  if (!value)
    return ostream << "(null)";
  return ostream << *value;
}

}  // namespace analyzer
}  // namespace joana
