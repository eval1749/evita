// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "evita/visuals/model/text_box.h"

#include "evita/visuals/fonts/text_format.h"
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
  if (is_display_none() || text_.empty())
    return FloatSize();
  // TODO(eval1749): Implement computing preferred size for |TextBox|.
  const auto font_size = 15.0f;
  const auto& text_format = std::make_unique<TextFormat>(L"Arial", font_size);
  const auto& width = text_format->ComputeWidth(text_);
  return FloatSize(width, font_size);
  return FloatSize(width, font_size) + border().size() + padding().size();
}

}  // namespace visuals
