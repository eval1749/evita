// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_LAYOUT_TEXT_BOX_H_
#define EVITA_VISUALS_LAYOUT_TEXT_BOX_H_

#include "base/strings/string16.h"
#include "evita/visuals/layout/content_box.h"
#include "evita/visuals/css/values.h"

namespace visuals {

class TextFormat;

//////////////////////////////////////////////////////////////////////
//
// TextBox
//
class TextBox final : public ContentBox {
  DECLARE_VISUAL_BOX_FINAL_CLASS(TextBox, ContentBox);

 public:
  TextBox(RootBox* root_box, const base::string16 text, const Node* node);
  TextBox(RootBox* root_box, const base::string16 text);
  ~TextBox() final;

  float baseline() const { return baseline_; }
  const FloatColor& color() const { return color_; }
  const base::string16& text() const { return text_; }
  const TextFormat& text_format() const;

 private:
  float baseline_ = 0.0f;
  FloatColor color_;

  base::string16 text_;

  // CSS font properties
  css::FontFamily font_family_;
  css::FontSize font_size_;
  css::FontStretch font_stretch_;
  css::FontStyle font_style_;
  css::FontWeight font_weight_;

  // |BoxEditor| will set/reset |font_description_| whenever one of font related
  // CSS properties changed.
  mutable const TextFormat* text_format_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(TextBox);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_LAYOUT_TEXT_BOX_H_
