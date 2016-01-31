// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_GFX_TEXT_LAYOUT_H_
#define EVITA_GFX_TEXT_LAYOUT_H_

#include <dwrite.h>

#include "evita/gfx/forward.h"
#include "evita/gfx/simple_object.h"

namespace gfx {

//////////////////////////////////////////////////////////////////////
//
// TextLayout
//
class TextLayout final : public SimpleObject_<IDWriteTextLayout> {
 public:
  explicit TextLayout(IDWriteTextLayout* text_layout);
  ~TextLayout();

  gfx::SizeF ComputeSize() const;

 private:
  DISALLOW_COPY_AND_ASSIGN(TextLayout);
};

}  // namespace gfx

#endif  // EVITA_GFX_TEXT_LAYOUT_H_
