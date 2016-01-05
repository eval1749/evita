// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DOM_TEXT_H_
#define EVITA_VISUALS_DOM_TEXT_H_

#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "evita/visuals/dom/node.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// Text
//
class Text final : public Node {
  DECLARE_VISUAL_NODE_FINAL_CLASS(Text, Node);

 public:
  Text(Document* document, const base::StringPiece16& data);
  ~Text() final;

  const base::string16& data() const { return data_; }

 private:
  base::string16 data_;

  DISALLOW_COPY_AND_ASSIGN(Text);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_DOM_TEXT_H_
