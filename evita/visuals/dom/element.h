// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DOM_ELEMENT_H_
#define EVITA_VISUALS_DOM_ELEMENT_H_

#include "evita/visuals/dom/element_node.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// Element
//
class Element final : public ElementNode {
  DECLARE_VISUAL_NODE_FINAL_CLASS(Element, ElementNode);

 public:
  Element(Document* document,
          base::StringPiece16 tag_name,
          base::StringPiece16 id);
  Element(Document* document, base::StringPiece16 tag_name);
  ~Element() final;

 private:
  DISALLOW_COPY_AND_ASSIGN(Element);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_DOM_ELEMENT_H_
