// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_gfx_text_format_h)
#define INCLUDE_evita_gfx_text_format_h

#include "evita/gfx_base.h"

namespace gfx {

class TextFormat : public SimpleObject_<IDWriteTextFormat> {
  private: const scoped_refptr<FactorySet> factory_set_;
  public: TextFormat(const base::string16& font_face_name, float font_size);
  public: TextFormat(const LOGFONT& log_font);
  public: std::unique_ptr<TextLayout> CreateLayout(
      const base::string16& text, const SizeF& size) const;
  public: float GetWidth(const base::string16& text) const;
  DISALLOW_COPY_AND_ASSIGN(TextFormat);
};

} // namespace gfx

#endif //!defined(INCLUDE_evita_gfx_text_format_h)
