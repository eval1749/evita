// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_LAYOUT_TEXT_BOX_H_
#define EVITA_VISUALS_LAYOUT_TEXT_BOX_H_

#include <memory>

#include "base/strings/string_piece.h"
#include "evita/visuals/layout/content_box.h"
#include "evita/visuals/css/values.h"

namespace visuals {

class TextFormat;
class TextLayout;

//////////////////////////////////////////////////////////////////////
//
// TextBox
//
class TextBox final : public ContentBox {
  DECLARE_VISUAL_BOX_FINAL_CLASS(TextBox, ContentBox);

 public:
  TextBox(RootBox* root_box, const base::StringPiece16& data, const Node* node);
  TextBox(RootBox* root_box, const base::StringPiece16& data);
  ~TextBox() final;

  float baseline() const { return baseline_; }
  const FloatColor& color() const { return color_; }
  const base::string16& data() const { return data_; }
  bool has_text_layout() const { return !!text_layout_; }
  const TextFormat& text_format() const;
  const TextLayout& text_layout() const;

 private:
  // Box
  void TextBox::DidChangeBounds(const FloatRect& old_bounds) final;

  float baseline_ = 0.0f;
  FloatColor color_;
  base::string16 data_;

  // CSS font properties
  css::FontFamily font_family_;
  css::FontSize font_size_;
  css::FontStretch font_stretch_;
  css::FontStyle font_style_;
  css::FontWeight font_weight_;

  // |BoxEditor::EnsureTextFormat()| will set/reset |font_description_| whenever
  // one of font related CSS properties changed.
  const TextFormat* text_format_ = nullptr;

  // |Layouter| allocates |TextLayout|.
  std::unique_ptr<TextLayout> text_layout_;

  DISALLOW_COPY_AND_ASSIGN(TextBox);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_LAYOUT_TEXT_BOX_H_
