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

ContainerBox::ContainerBox(RootBox* root_box, const Node* node)
    : Box(root_box, node) {}

ContainerBox::ContainerBox(RootBox* root_box) : Box(root_box) {}

ContainerBox::~ContainerBox() {
  DCHECK_EQ(static_cast<ContainerBox*>(nullptr), parent());
  auto runner = first_child_;
  while (runner) {
    const auto next_child = runner->next_sibling();
    BoxEditor().WillDestroy(runner);
    delete runner;
    runner = next_child;
  }
}

}  // namespace visuals
