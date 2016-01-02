// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DOM_TEXT_NODE_H_
#define EVITA_VISUALS_DOM_TEXT_NODE_H_

#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "evita/visuals/dom/node.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// TextNode
//
class TextNode final : public Node {
  DECLARE_VISUAL_NODE_FINAL_CLASS(TextNode, Node);

 public:
  TextNode(Document* document, const base::StringPiece16& text);
  ~TextNode() final;

  const base::string16& text() const { return text_; }

 private:
  base::string16 text_;

  DISALLOW_COPY_AND_ASSIGN(TextNode);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_DOM_TEXT_NODE_H_
