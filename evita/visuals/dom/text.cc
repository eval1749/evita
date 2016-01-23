// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/dom/text.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// Text
//
Text::Text(Document* document, base::StringPiece16 data)
    : Node(document, L"#text"), data_(data.as_string()) {}

Text::~Text() {}

}  // namespace visuals
