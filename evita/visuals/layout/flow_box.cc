// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/layout/flow_box.h"

namespace visuals {

FlowBox::FlowBox(RootBox* root_box, const Node* node)
    : ContainerBox(root_box, node) {}

FlowBox::FlowBox(RootBox* root_box) : ContainerBox(root_box) {}

FlowBox::~FlowBox() {}

}  // namespace visuals
