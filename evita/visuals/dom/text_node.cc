// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "evita/visuals/dom/text_node.h"

#include "evita/visuals/dom/node_editor.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// TextNode
//
TextNode::TextNode(Document* document, const base::StringPiece16& text)
    : Node(document, L"#text"), text_(text.as_string()) {
  NodeEditor().SetContentChanged(this);
}

TextNode::~TextNode() {}

}  // namespace visuals
