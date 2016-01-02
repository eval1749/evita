// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "evita/visuals/model/line_box.h"

namespace visuals {

LineBox::LineBox(RootBox* root_box, const base::StringPiece16& id)
    : ContainerBox(root_box, id) {}

LineBox::LineBox(RootBox* root_box) : ContainerBox(root_box) {}
LineBox::~LineBox() {}

}  // namespace visuals
