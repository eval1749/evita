// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "evita/visuals/model/inline_flow_box.h"

namespace visuals {

InlineFlowBox::InlineFlowBox(RootBox* root_box, const Node* node)
    : ContainerBox(root_box, node) {}

InlineFlowBox::InlineFlowBox(RootBox* root_box) : ContainerBox(root_box) {}
InlineFlowBox::~InlineFlowBox() {}

}  // namespace visuals
