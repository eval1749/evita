// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/dom/image.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// Image
//
Image::Image(Document* document, const ImageData& data)
    : Node(document, L"#image"), data_(data) {}

Image::~Image() {}

}  // namespace visuals
