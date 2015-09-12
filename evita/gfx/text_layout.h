// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_GFX_TEXT_LAYOUT_H_
#define EVITA_GFX_TEXT_LAYOUT_H_

#include "evita/gfx_base.h"

namespace gfx {

class TextLayout final : public SimpleObject_<IDWriteTextLayout> {
 public:
  explicit TextLayout(IDWriteTextLayout* text_layout);
  ~TextLayout();

  SIZE GetMetrics() const;

 private:
  DISALLOW_COPY_AND_ASSIGN(TextLayout);
};

}  // namespace gfx

#endif  // EVITA_GFX_TEXT_LAYOUT_H_
