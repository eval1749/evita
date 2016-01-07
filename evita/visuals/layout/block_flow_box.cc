// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "evita/visuals/layout/block_flow_box.h"

namespace visuals {

BlockFlowBox::BlockFlowBox(RootBox* root_box, const Node* node)
    : ContainerBox(root_box, node) {}

BlockFlowBox::BlockFlowBox(RootBox* root_box) : ContainerBox(root_box) {}

BlockFlowBox::~BlockFlowBox() {}

}  // namespace visuals
