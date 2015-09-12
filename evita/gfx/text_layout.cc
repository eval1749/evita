// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/gfx/text_layout.h"

namespace gfx {

//////////////////////////////////////////////////////////////////////
//
// TextLayout
//
TextLayout::TextLayout(IDWriteTextLayout* text_layout)
    : SimpleObject_(text_layout) {}

TextLayout::~TextLayout() {}

}  // namespace gfx
