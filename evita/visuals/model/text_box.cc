// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/model/text_box.h"

#include "evita/visuals/model/box_editor.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// TextBox
//
TextBox::TextBox(RootBox* root_box, const base::string16 text)
    : InlineBox(root_box), text_(text) {
  BoxEditor().SetContentChanged(this);
}

TextBox::~TextBox() {}

FloatSize TextBox::ComputePreferredSize() const {
  if (is_display_none())
    return FloatSize();
  // TODO(eval1749): Implement computing preferred size for |TextBox|.
  return FloatSize(text_.size() * 8.0f, 50) + border().size() +
         padding().size();
}

}  // namespace visuals
