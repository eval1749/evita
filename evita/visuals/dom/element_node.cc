// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/dom/element_node.h"

#include "evita/visuals/css/style.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// ElementNode
//
ElementNode::ElementNode(Document* document,
                         base::StringPiece16 tag_name,
                         base::StringPiece16 id)
    : ContainerNode(document, tag_name, id) {}

ElementNode::ElementNode(Document* document, base::StringPiece16 tag_name)
    : ElementNode(document, tag_name, base::StringPiece16()) {}

ElementNode::~ElementNode() {}

}  // namespace visuals
