// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/layout/shape_box.h"

#include "evita/visuals/layout/box_editor.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// ShapeBox
//
ShapeBox::ShapeBox(RootBox* root_box, const ShapeData& data, const Node* node)
    : ContentBox(root_box, node), data_(data) {
  BoxEditor().SetContentChanged(this);
}

ShapeBox::~ShapeBox() {}

}  // namespace visuals
