// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "evita/visuals/dom/element.h"

namespace visuals {

Element::Element(Document* document,
                 const base::StringPiece16& tag_name,
                 const base::StringPiece16& id)
    : ContainerNode(document, tag_name, id) {}

Element::Element(Document* document, const base::StringPiece16& tag_name)
    : Element(document, tag_name, base::StringPiece16()) {}
Element::~Element() {}

}  // namespace visuals
