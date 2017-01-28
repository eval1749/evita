// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_VALUES_H_
#define JOANA_ANALYZER_VALUES_H_

#include <iosfwd>
#include <vector>

#include "joana/analyzer/value.h"
#include "joana/base/block_range.h"
#include "joana/base/iterator_utils.h"
#include "joana/base/memory/zone_vector.h"

namespace joana {
namespace analyzer {

class Function;
class Properties;
class Type;
class TypeParameter;

//
// ClassKind
//
enum class ClassKind {
  Class,
  Interface,
  Record,
};

std::ostream& operator<<(std::ostream& ostream, ClassKind kind);

//
// VariableKind
//
#define FOR_EACH_VARIABLE_KIND(V) \
  V(Catch)                        \
  V(Class)                        \
  V(Const)                        \
  V(Function)                     \
  V(Let)                          \
  V(Parameter)                    \
  V(Var)

enum class VariableKind {
#define V(name) name,
  FOR_EACH_VARIABLE_KIND(V)
#undef V
};

std::ostream& operator<<(std::ostream& ostream, VariableKind kind);

//
// Visibility
//
#define FOR_EACH_VISIBILITY(V) \
  V(Private)                   \
  V(Protected)                 \
  V(Public)

enum class Visibility {
#define V(name) name,
  FOR_EACH_VISIBILITY(V)
#undef V
};

std::ostream& operator<<(std::ostream& ostream, Visibility visibility);

//
// Object
//
class Object : public Value {
  DECLARE_ABSTRACT_ANALYZE_VALUE(Object, Value);

 public:
  ~Object() override;

  const Properties& properties() const { return properties_; }
  Properties& properties() { return properties_; }

 protected:
  Object(int id, const ast::Node& node, Properties* properties);

 private:
  Properties& properties_;

  DISALLOW_COPY_AND_ASSIGN(Object);
};

//
// ValueHolder
//
class ValueHolder : public Object {
  DECLARE_ABSTRACT_ANALYZE_VALUE(ValueHolder, Object);

 public:
  ~ValueHolder() override;

  const ZoneVector<const ast::Node*>& assignments() const {
    return assignments_;
  }

  const ZoneVector<const ast::Node*>& references() const { return references_; }

 protected:
  ValueHolder(Zone* zone,
              int id,
              const ast::Node& node,
              Properties* properties);

 private:
  ZoneVector<const ast::Node*> assignments_;
  ZoneVector<const ast::Node*> references_;

  DISALLOW_COPY_AND_ASSIGN(ValueHolder);
};

//
// Class
//
class Class : public Object {
  DECLARE_CONCRETE_ANALYZE_VALUE(Class, Object);

 public:
  ~Class() final;

  ClassKind kind() const { return kind_; }
  auto parameters() const { return ReferenceRangeOf(parameters_); }

 protected:
  // TODO(eval1749): move |zone| after |node|.
  Class(Zone* zone,
        int id,
        const ast::Node& node,
        ClassKind kind,
        const std::vector<const TypeParameter*>& parameters,
        Properties* properties);

 private:
  const ClassKind kind_;
  ZoneVector<const TypeParameter*> parameters_;

  DISALLOW_COPY_AND_ASSIGN(Class);
};

//
// ConstructedClass
//
class ConstructedClass : public Value {
  DECLARE_CONCRETE_ANALYZE_VALUE(ConstructedClass, Value);

 public:
  void* operator new(size_t size, void* pointer) { return pointer; }

  ~ConstructedClass() final;

  BlockRange<const Type*> arguments() const;
  Class& generic_class() const { return generic_class_; }

 private:
  ConstructedClass(int id,
                   Class* generic_class,
                   const std::vector<const Type*>& arguments);

  Class& generic_class_;
  const size_t number_of_arguments_;
  const Type* arguments_[1];

  DISALLOW_COPY_AND_ASSIGN(ConstructedClass);
};

//
// Function
//
class Function final : public Object {
  DECLARE_CONCRETE_ANALYZE_VALUE(Function, Value)

 public:
  ~Function() final;

 private:
  // |properties| is used for holding members of anonymous class
  Function(int id, const ast::Node& node, Properties* properties);

  DISALLOW_COPY_AND_ASSIGN(Function);
};

//
// OrdinaryObject
//
class OrdinaryObject final : public Object {
  DECLARE_CONCRETE_ANALYZE_VALUE(OrdinaryObject, Object)

 public:
  ~OrdinaryObject() final;

 private:
  OrdinaryObject(int id, const ast::Node& node, Properties* properties);

  DISALLOW_COPY_AND_ASSIGN(OrdinaryObject);
};

//
// Property
//
class Property final : public ValueHolder {
  DECLARE_CONCRETE_ANALYZE_VALUE(Property, ValueHolder)

 public:
  ~Property();

  const ast::Node& key() const { return node(); }
  Visibility visibility() const { return visibility_; }

 private:
  Property(Zone* zone,
           int id,
           Visibility visibility,
           const ast::Node& key,
           Properties* properties);

  const Visibility visibility_;

  DISALLOW_COPY_AND_ASSIGN(Property);
};

//
// Undefined
//
class Undefined final : public Value {
  DECLARE_CONCRETE_ANALYZE_VALUE(Undefined, Value)

 public:
  ~Undefined() final;

 private:
  Undefined(int id, const ast::Node& node);

  DISALLOW_COPY_AND_ASSIGN(Undefined);
};

//
// Variable
//
class Variable final : public ValueHolder {
  DECLARE_CONCRETE_ANALYZE_VALUE(Variable, ValueHolder)

 public:
  ~Variable() final;

  VariableKind kind() const { return kind_; }

 private:
  // |name| should be |ast::Name| or |ast::BindingNameElement|.
  Variable(Zone* zone,
           int id,
           VariableKind kind,
           const ast::Node& name,
           Properties* properties);

  const VariableKind kind_;

  DISALLOW_COPY_AND_ASSIGN(Variable);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_VALUES_H_
