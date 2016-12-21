// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_IR_PRIMITIVE_TYPE_FACTORY_H_
#define JOANA_IR_PRIMITIVE_TYPE_FACTORY_H_

#include "base/macros.h"
#include "joana/base/memory/zone.h"
#include "joana/ir/ir_export.h"
#include "joana/ir/type_forward.h"

namespace base {
template <typename T>
struct DefaultSingletonTraits;
}

namespace joana {

class Zone;

namespace ir {

//
// PrimitiveTypeFactory
//
class PrimitiveTypeFactory final {
 public:
  static PrimitiveTypeFactory* GetInstance();

#define V(capital, underscore) \
  const Type& underscore##_type() const { return underscore##_type_; }
  FOR_EACH_IR_PRIMITIVE_TYPE(V)
#undef V

 private:
  friend struct base::DefaultSingletonTraits<PrimitiveTypeFactory>;

  PrimitiveTypeFactory();
  ~PrimitiveTypeFactory() = default;

  Zone zone_;

#define V(capital, underscore) const Type& underscore##_type_;
  FOR_EACH_IR_PRIMITIVE_TYPE(V)
#undef V
  int dummy_;

  DISALLOW_COPY_AND_ASSIGN(PrimitiveTypeFactory);
};

}  // namespace ir
}  // namespace joana

#endif  // JOANA_IR_PRIMITIVE_TYPE_FACTORY_H_
