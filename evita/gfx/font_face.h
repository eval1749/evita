// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_gfx_font_face_h)
#define INCLUDE_evita_gfx_font_face_h

#include "evita/gfx_base.h"

namespace gfx {

class FontFace : public SimpleObject_<IDWriteFontFace> {
  private: const DWRITE_FONT_METRICS metrics_;
  public: FontFace(const char16* family_name);
  public: ~FontFace();
  public: const DWRITE_FONT_METRICS& metrics() const { return metrics_; }
  DISALLOW_COPY_AND_ASSIGN(FontFace);
};

} // namespace gfx

#endif //!defined(INCLUDE_evita_gfx_font_face_h)
