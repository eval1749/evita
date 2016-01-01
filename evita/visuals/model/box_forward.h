// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_MODEL_BOX_FORWARD_H_
#define EVITA_VISUALS_MODEL_BOX_FORWARD_H_

namespace visuals {

#define FOR_EACH_VISUAL_BOX(V) \
  V(BlockBox)                  \
  V(LineBox)                   \
  V(RootBox)                   \
  V(TextBox)

#define FOR_EACH_ABSTRACT_VISUAL_BOX(V) \
  V(Box)                                \
  V(ContainerBox)                       \
  V(InlineBox)

// Forward declarations
class Box;
#define V(name) class name;
FOR_EACH_VISUAL_BOX(V)
FOR_EACH_ABSTRACT_VISUAL_BOX(V)
#undef V
class BoxVisitor;

}  // namespace visuals

#endif  // EVITA_VISUALS_MODEL_BOX_FORWARD_H_
