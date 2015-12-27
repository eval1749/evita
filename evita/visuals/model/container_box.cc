// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/model/container_box.h"

#include "base/logging.h"
#include "evita/visuals/model/box_editor.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// ContainerBox
//
ContainerBox::ContainerBox() {}

ContainerBox::~ContainerBox() {
  DCHECK_EQ(static_cast<ContainerBox*>(nullptr), parent());
  for (const auto& child : child_boxes_) {
    BoxEditor().SetParent(child, nullptr);
    delete child;
  }
}

}  // namespace visuals
