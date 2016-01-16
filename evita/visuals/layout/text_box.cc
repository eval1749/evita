// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "evita/visuals/layout/text_box.h"

#include "evita/visuals/fonts/text_layout.h"
#include "evita/visuals/layout/box_editor.h"
#include "evita/visuals/layout/root_box.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// TextBox
//
TextBox::TextBox(RootBox* root_box, const base::string16 text, const Node* node)
    : ContentBox(root_box, node), text_(text) {
  BoxEditor().SetContentChanged(this);
}

TextBox::TextBox(RootBox* root_box, const base::string16 text)
    : TextBox(root_box, text, nullptr) {}

TextBox::~TextBox() {}

const TextFormat& TextBox::text_format() const {
  DCHECK(text_format_);
  return *text_format_;
}

const TextLayout& TextBox::text_layout() const {
  DCHECK(root_box()->InPaint()) << root_box()->lifecycle();
  DCHECK(text_layout_);
  return *text_layout_;
}

void TextBox::DidChangeBounds(const FloatRect& old_bounds) {
  DCHECK(root_box()->InLayout()) << root_box()->lifecycle();
  text_layout_.reset();
}

}  // namespace visuals
