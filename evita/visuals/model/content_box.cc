// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/model/content_box.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// ContentBox
//
ContentBox::ContentBox(RootBox* root_box, const Node* node)
    : Box(root_box, node) {}

ContentBox::ContentBox(RootBox* root_box) : ContentBox(root_box, nullptr) {}

ContentBox::~ContentBox() {}

}  // namespace visuals
