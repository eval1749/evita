// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_MODEL_TEXT_BOX_H_
#define EVITA_VISUALS_MODEL_TEXT_BOX_H_

#include "base/strings/string16.h"
#include "evita/visuals/model/inline_box.h"
#include "evita/visuals/style/float_color.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// TextBox
//
class TextBox final : public InlineBox {
  DECLARE_VISUAL_BOX_FINAL_CLASS(TextBox, InlineBox);

 public:
  explicit TextBox(const base::string16 text);
  ~TextBox() final;

  const FloatColor& color() const { return color_; }
  float baseline() const { return baseline_; }
  const base::string16& text() const { return text_; }

 private:
  // Box
  FloatSize ComputePreferredSize() const final;

  float baseline_ = 0.0f;
  FloatColor color_;
  base::string16 text_;

  DISALLOW_COPY_AND_ASSIGN(TextBox);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_MODEL_TEXT_BOX_H_
