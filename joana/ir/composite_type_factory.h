// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_IR_COMPOSITE_TYPE_FACTORY_H_
#define JOANA_IR_COMPOSITE_TYPE_FACTORY_H_

#include <memory>
#include <vector>

#include "base/macros.h"
#include "joana/base/memory/zone.h"
#include "joana/ir/ir_export.h"
#include "joana/ir/type_forward.h"

namespace joana {

class Zone;

namespace ir {

//
// CompositeTypeFactory
//
class CompositeTypeFactory final {
 public:
  explicit CompositeTypeFactory(Zone* zone);
  ~CompositeTypeFactory();

  const Type& NewFunctionType(const Type& parameters_type,
                              const Type& return_type);

  const Type& NewReferenceType(const Type& to);

  const Type& NewTupleType(const std::vector<const Type*>& members);

 private:
  class Cache;

  std::unique_ptr<Cache> cache_;
  Zone& zone_;

  DISALLOW_COPY_AND_ASSIGN(CompositeTypeFactory);
};

}  // namespace ir
}  // namespace joana

#endif  // JOANA_IR_COMPOSITE_TYPE_FACTORY_H_
