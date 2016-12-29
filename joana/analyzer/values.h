// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_VALUES_H_
#define JOANA_ANALYZER_VALUES_H_

#include "joana/analyzer/value.h"
#include "joana/base/memory/zone_vector.h"

namespace joana {
namespace analyzer {

//
// LexicalBinding
//
class LexicalBinding : public Value {
  DECLARE_ABSTRACT_ANALYZE_VALUE(LexicalBinding, Value);

 public:
  ~LexicalBinding() override;

  const ZoneVector<const ast::Node*>& assignments() const {
    return assignments_;
  }

  const ZoneVector<const ast::Node*>& references() const { return references_; }

 protected:
  LexicalBinding(Zone* zone, const ast::Node& node);

 private:
  ZoneVector<const ast::Node*> assignments_;
  ZoneVector<const ast::Node*> references_;

  DISALLOW_COPY_AND_ASSIGN(LexicalBinding);
};

//
// Function
//
class Function final : public LexicalBinding {
  DECLARE_CONCRETE_ANALYZE_VALUE(Function, LexicalBinding)

 public:
  ~Function() final;

 private:
  Function(Zone* zone, const ast::Node& node);

  DISALLOW_COPY_AND_ASSIGN(Function);
};

//
// Property
//
class Property final : public LexicalBinding {
  DECLARE_CONCRETE_ANALYZE_VALUE(Property, LexicalBinding)

 public:
  ~Property() final;

 private:
  Property(Zone* zone, const ast::Node& node);

  DISALLOW_COPY_AND_ASSIGN(Property);
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
  Variable(Zone* zone, const ast::Node& assignment, const ast::Node& name);

  const ast::Node& assignment_;

  DISALLOW_COPY_AND_ASSIGN(Variable);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_VALUES_H_
