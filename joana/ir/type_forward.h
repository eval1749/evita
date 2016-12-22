// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_IR_TYPE_FORWARD_H_
#define JOANA_IR_TYPE_FORWARD_H_

namespace joana {
namespace ir {

#define FOR_EACH_IR_ABSTRACT_TYPE(V) \
  V(Composite)                       \
  V(Primitive)                       \
  V(Type)

// Composite types:
//  Class
//  Function
//  GenericClass
//  GenericFunction
//  JsEnum
//  JsArray
//  JsObject
//  Reference
//  Tuple
//  Union
#define FOR_EACH_IR_COMPOSITE_TYPE(V) \
  V(Function)                         \
  V(Reference)                        \
  V(Tuple)                            \
  V(Union)

#define FOR_EACH_IR_PRIMITIVE_TYPE(V) \
  V(Any, any)                         \
  V(Bool, bool)                       \
  V(Control, control)                 \
  V(Effect, effect)                   \
  V(Float64, float64)                 \
  V(Int64, int64)                     \
  V(JsNull, js_null)                  \
  V(JsSymbol, js_symbol)              \
  V(Nil, nil)                         \
  V(String, string) /* NOLINT */      \
  V(Void, void)

#define FOR_EACH_IR_TYPE(V)     \
  FOR_EACH_IR_COMPOSITE_TYPE(V) \
  FOR_EACH_IR_PRIMITIVE_TYPE(V)

class Type;
#define V(capital, ...) class capital##Type;
FOR_EACH_IR_TYPE(V)
#undef V

}  // namespace ir
}  // namespace joana

#endif  // JOANA_IR_TYPE_FORWARD_H_
