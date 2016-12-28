// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_VALUES_H_
#define JOANA_ANALYZER_VALUES_H_

#include "joana/analyzer/value.h"

namespace joana {
namespace analyzer {

//
// Function
//
class Function final : public Value {
  DECLARE_CONCRETE_ANALYZE_VALUE(Function, Value)
 public:
  ~Function() final;

 private:
  explicit Function(const ast::Node& node);

  // List of assignment
  // List of uses

  DISALLOW_COPY_AND_ASSIGN(Function);
};

//
// Property
//
class Property final : public Value {
  DECLARE_CONCRETE_ANALYZE_VALUE(Property, Value)
 public:
  ~Property() final;

 private:
  explicit Property(const ast::Node& node);

  // List of assignment
  // List of uses

  DISALLOW_COPY_AND_ASSIGN(Property);
};

//
// Variable
//
class Variable final : public Value {
  DECLARE_CONCRETE_ANALYZE_VALUE(Variable, Value)
 public:
  ~Variable() final;

 private:
  explicit Variable(const ast::Node& node);

  // List of assignment
  // List of uses

  DISALLOW_COPY_AND_ASSIGN(Variable);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_VALUES_H_
