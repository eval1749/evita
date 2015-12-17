// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_GFX_TEXT_FORMAT_H_
#define EVITA_GFX_TEXT_FORMAT_H_

#include "evita/gfx_base.h"

namespace gfx {

class TextFormat final : public SimpleObject_<IDWriteTextFormat> {
 public:
  TextFormat(const base::string16& font_face_name, float font_size);
  explicit TextFormat(const LOGFONT& log_font);

  std::unique_ptr<TextLayout> CreateLayout(const base::string16& text,
                                           const SizeF& size) const;
  float GetWidth(const base::string16& text) const;

 private:
  DISALLOW_COPY_AND_ASSIGN(TextFormat);
};

}  // namespace gfx

#endif  // EVITA_GFX_TEXT_FORMAT_H_
