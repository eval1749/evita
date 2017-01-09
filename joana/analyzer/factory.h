// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_FACTORY_H_
#define JOANA_ANALYZER_FACTORY_H_

#include "base/macros.h"

namespace joana {

namespace ast {
class Node;
}
class Zone;

namespace analyzer {

class Class;
enum class ClassKind;
class Function;
class Environment;
class Properties;
class Property;
class Type;
class Value;
class Variable;

//
// Factory
//
class Factory final {
 public:
  // |zone| A zone to store analyze results.
  explicit Factory(Zone* zone);
  ~Factory();

  // Factory members
  Environment& NewEnvironment(Environment* outer, const ast::Node& owner);

  Property& GetOrNewProperty(Properties* properties, const ast::Node& node);
  Property& NewProperty(const ast::Node& node);

  // Values
  Class& NewClass(const ast::Node& node, ClassKind kind);
  Function& NewFunction(const ast::Node& node);
  Value& NewOrdinaryObject(const ast::Node& node);
  Value& NewUndefined(const ast::Node& node);
  Variable& NewVariable(const ast::Node& name);

  void ResetValueId();

 private:
  int NextValueId();

  // Properties
  Properties& NewProperties(const ast::Node& node);

  int current_value_id_ = 0;
  Zone& zone_;

  DISALLOW_COPY_AND_ASSIGN(Factory);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_FACTORY_H_
