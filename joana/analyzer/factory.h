// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_FACTORY_H_
#define JOANA_ANALYZER_FACTORY_H_

#include <memory>
#include <vector>

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
class GenericClass;
class Properties;
class Property;
class Type;
class TypeParameter;
class Value;
class ValueHolderData;
class Variable;
enum class VariableKind;
enum class Visibility;

//
// Factory
//
class Factory final {
 public:
  // |zone| A zone to store analyze results.
  explicit Factory(Zone* zone);
  ~Factory();

  // Factory members
  const Class& NewConstructedClass(const GenericClass& generic_class,
                                   const std::vector<const Type*>& arguments);

  const Function& NewFunction(const ast::Node& name, const ast::Node& node);

  const Class& NewGenericClass(
      ClassKind kind,
      const ast::Node& name,
      const ast::Node& node,
      const std::vector<const TypeParameter*>& parameters,
      Properties* properties);

  const Class& NewNormalClass(ClassKind kind,
                              const ast::Node& name,
                              const ast::Node& node,
                              Properties* properties);

  const Value& NewOrdinaryObject(const ast::Node& node, Properties* properties);

  Properties& NewProperties(const ast::Node& node);

  const Property& NewProperty(Visibility visibility,
                              const ast::Node& node,
                              ValueHolderData* data,
                              Properties* properties);

  const Value& NewUndefined(const ast::Node& node);

  ValueHolderData& NewValueHolderData();

  const Variable& NewVariable(VariableKind kind,
                              const ast::Node& name,
                              ValueHolderData* data,
                              Properties* properties);

  void ResetCurrentId();
  void ResetCurrentIdForTesting(int current_id);

 private:
  class Cache;

  int NextValueId();

  const std::unique_ptr<Cache> cache_;
  int current_value_id_ = 0;
  Zone& zone_;

  DISALLOW_COPY_AND_ASSIGN(Factory);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_FACTORY_H_
