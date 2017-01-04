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
// LexicalBinding
//
class LexicalBinding : public Object {
  DECLARE_ABSTRACT_ANALYZE_VALUE(LexicalBinding, Object);

 public:
  ~LexicalBinding() override;

  const ZoneVector<const ast::Node*>& assignments() const {
    return assignments_;
  }

  const ZoneVector<const ast::Node*>& references() const { return references_; }

 protected:
  LexicalBinding(Zone* zone,
                 int id,
                 const ast::Node& node,
                 Properties* properties);

 private:
  ZoneVector<const ast::Node*> assignments_;
  ZoneVector<const ast::Node*> references_;

  DISALLOW_COPY_AND_ASSIGN(LexicalBinding);
};

//
// Class
//
class Class final : public LexicalBinding {
  DECLARE_CONCRETE_ANALYZE_VALUE(Class, LexicalBinding)

 public:
  ~Class() final;

  const Object& prototype() const { return prototype_; }
  Object& prototype() { return prototype_; }

 private:
  Class(Zone* zone,
        int id,
        const ast::Node& node,
        Properties* properties,
        Object* prototype);

  // The prototype
  Object& prototype_;

  DISALLOW_COPY_AND_ASSIGN(Class);
};

//
// Function
//
class Function final : public LexicalBinding {
  DECLARE_CONCRETE_ANALYZE_VALUE(Function, LexicalBinding)

 public:
  ~Function() final;

 private:
  Function(Zone* zone, int id, const ast::Node& node, Properties* properties);

  DISALLOW_COPY_AND_ASSIGN(Function);
};

//
// Method
//
class Method final : public LexicalBinding {
  DECLARE_CONCRETE_ANALYZE_VALUE(Method, LexicalBinding)

 public:
  ~Method() final;

  Class& owner() const { return owner_; }

 private:
  Method(Zone* zone,
         int id,
         const ast::Node& node,
         Class* owner,
         Properties* properties);

  Class& owner_;

  DISALLOW_COPY_AND_ASSIGN(Method);
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
class Variable final : public LexicalBinding {
  DECLARE_CONCRETE_ANALYZE_VALUE(Variable, LexicalBinding)

 public:
  ~Variable() final;

  const ast::Node& assignment() const { return assignment_; }

 private:
  // |name| should be |ast::Name| or |ast::BindingNameElement|.
  Variable(Zone* zone,
           int id,
           const ast::Node& assignment,
           const ast::Node& name,
           Properties* properties);

  const ast::Node& assignment_;

  DISALLOW_COPY_AND_ASSIGN(Variable);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_VALUES_H_
