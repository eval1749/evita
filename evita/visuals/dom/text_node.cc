// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "evita/visuals/dom/text_node.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// TextNode
//
TextNode::TextNode(Document* document, const base::StringPiece16& data)
    : Node(document, L"#text"), data_(data.as_string()) {}

TextNode::~TextNode() {}

}  // namespace visuals
