// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/model/inline_box.h"

namespace visuals {

InlineBox::InlineBox(RootBox* root_box, const Node* node)
    : ContainerBox(root_box, node) {}

InlineBox::InlineBox(RootBox* root_box) : ContainerBox(root_box) {}
InlineBox::~InlineBox() {}

}  // namespace visuals
