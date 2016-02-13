// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/layout/image_box.h"

#include "evita/visuals/imaging/image_bitmap.h"
#include "evita/visuals/layout/box_editor.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// ImageBox
//
ImageBox::ImageBox(RootBox* root_box, const ImageData& data, const Node* node)
    : ContentBox(root_box, node), data_(data) {
  BoxEditor().SetContentChanged(this);
}

ImageBox::~ImageBox() {}

}  // namespace visuals
