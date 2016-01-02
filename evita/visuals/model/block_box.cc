// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "evita/visuals/model/block_box.h"

namespace visuals {

BlockBox::BlockBox(RootBox* root_box, const base::StringPiece16& id)
    : ContainerBox(root_box, id) {}

BlockBox::BlockBox(RootBox* root_box) : ContainerBox(root_box) {}

BlockBox::~BlockBox() {}

}  // namespace visuals
