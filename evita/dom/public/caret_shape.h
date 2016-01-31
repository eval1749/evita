// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_PUBLIC_CARET_SHAPE_H_
#define EVITA_DOM_PUBLIC_CARET_SHAPE_H_

#include <iosfwd>

namespace domapi {

#define FOR_EACH_DOMAPI_CARET_SHAPE(V) \
  V(None)                              \
  V(Bar)                               \
  V(Box)

enum class CaretShape {
#define V(Name) Name,
  FOR_EACH_DOMAPI_CARET_SHAPE(V)
#undef V
};

std::ostream& operator<<(std::ostream& ostream, CaretShape caret_shape);

}  // namespace domapi

#endif  // EVITA_DOM_PUBLIC_CARET_SHAPE_H_
