// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_LAYOUT_BOX_TRAVERSAL_H_
#define EVITA_VISUALS_LAYOUT_BOX_TRAVERSAL_H_

#include "base/macros.h"

namespace visuals {

class Box;
class ContainerBox;

//////////////////////////////////////////////////////////////////////
//
// BoxTraversal
//
class BoxTraversal {
 public:
  static Box* FirstChildOf(const Box& box);
  static Box* LastChildOf(const Box& box);
  static Box* NextOf(const Box& box);
  static Box* NextSiblingOf(const Box& box);
  static ContainerBox* ParentOf(const Box& box);
  static Box* PreviousSiblingOf(const Box& box);

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(BoxTraversal);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_LAYOUT_BOX_TRAVERSAL_H_
