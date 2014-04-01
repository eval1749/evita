// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_gfx_text_layout_h)
#define INCLUDE_evita_gfx_text_layout_h

#include "evita/gfx_base.h"

namespace gfx {

class TextLayout : public SimpleObject_<IDWriteTextLayout> {
  public: TextLayout(IDWriteTextLayout* text_layout);
  public: ~TextLayout();
  public: SIZE GetMetrics() const;
  DISALLOW_COPY_AND_ASSIGN(TextLayout);
};

} // namespace gfx

#endif //!defined(INCLUDE_evita_gfx_text_layout_h)
