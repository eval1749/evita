// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/model/text_box.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// TextBox
//
TextBox::TextBox(const FloatRect& bounds, const base::string16 text)
    : InlineBox(bounds), text_(text) {}

TextBox::~TextBox() {}

}  // namespace visuals
