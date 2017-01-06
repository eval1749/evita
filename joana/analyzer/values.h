// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_VALUES_H_
#define JOANA_ANALYZER_VALUES_H_

#include "joana/analyzer/value.h"
#include "joana/base/memory/zone_vector.h"

namespace joana {
namespace analyzer {

class Environment;
class Object;
class Properties;

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
class Property final : public Object {
  DECLARE_CONCRETE_ANALYZE_VALUE(Property, Object)

 public:
  ~Property();

  const ZoneVector<const ast::Node*>& assignments() const {
    return assignments_;
  }

  const ast::Node& key() const { return key_; }

  const ZoneVector<const ast::Node*>& references() const { return references_; }

 private:
  Property(Zone* zone, int id, const ast::Node& key, Properties* properties);

  void AddAssignment(const ast::Node& assignment);
  void AddReference(const ast::Node& reference);

  ZoneVector<const ast::Node*> assignments_;
  const ast::Node& key_;
  ZoneVector<const ast::Node*> references_;

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

  const ast::Node& origin() const { return origin_; }

 private:
  // |name| should be |ast::Name| or |ast::BindingNameElement|.
  Variable(Zone* zone,
           int id,
           const ast::Node& origin,
           const ast::Node& name,
           Properties* properties);

  const ast::Node& origin_;

  DISALLOW_COPY_AND_ASSIGN(Variable);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_VALUES_H_
