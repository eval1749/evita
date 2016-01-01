// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/model/root_box.h"

#include "evita/visuals/model/box_traversal.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// RootBox
//
RootBox::RootBox() {}
RootBox::~RootBox() {}

// Box
FloatSize RootBox::ComputePreferredSize() const {
  return BoxTraversal::FirstChildOf(*this)->ComputePreferredSize();
}

}  // namespace visuals
