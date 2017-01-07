// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_TYPES_H_
#define JOANA_ANALYZER_TYPES_H_

#include "joana/analyzer/value.h"

namespace joana {
namespace analyzer {

//
// Type
//
class Type : public Value {
  DECLARE_ABSTRACT_ANALYZE_VALUE(Type, Value);

 public:
  ~Type() override;

 protected:
  Type(int id, const ast::Node& node);

 private:
  DISALLOW_COPY_AND_ASSIGN(Type);
};

//
// PrimitiveType
//
class PrimitiveType final : public Type {
  DECLARE_CONCRETE_ANALYZE_VALUE(PrimitiveType, Type)

 public:
  ~PrimitiveType() final;

 private:
  PrimitiveType(int id, const ast::Node& node);

  DISALLOW_COPY_AND_ASSIGN(PrimitiveType);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_TYPES_H_