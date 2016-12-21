// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_IR_TYPE_H_
#define JOANA_IR_TYPE_H_

#include <iosfwd>

#include "base/macros.h"
#include "joana/base/castable.h"
#include "joana/base/memory/zone_allocated.h"
#include "joana/ir/ir_export.h"
#include "joana/ir/type_forward.h"

namespace joana {
namespace ir {

class PrimitiveTypeFactory;
class TypeFactory;

#define DECLARE_IR_TYPE(name, base) DECLARE_CASTABLE_CLASS(name, base);

#define DECLARE_ABSTRACT_IR_TYPE(name, base) DECLARE_IR_TYPE(name, base);

#define DECLARE_CONCRETE_IR_TYPE(name, base) \
  DECLARE_IR_TYPE(name, base);               \
  friend class TypeFactory;

//
// Type
//
class JOANA_IR_EXPORT Type : public Castable<Type>, public ZoneAllocated {
  DECLARE_ABSTRACT_IR_TYPE(Type, Castable);

 public:
  ~Type();

  bool operator==(const Type& other) const;
  bool operator!=(const Type& other) const;

 protected:
  Type();

 private:
  DISALLOW_COPY_AND_ASSIGN(Type);
};

// Implemented in "joana/ir/type_printer.cc"
JOANA_IR_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                         const Type& type);

JOANA_IR_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                         const Type* type);

}  // namespace ir
}  // namespace joana

#endif  // JOANA_IR_TYPE_H_
