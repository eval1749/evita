// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/gfx/text_layout.h"

#include "evita/gfx/size_f.h"

namespace gfx {

//////////////////////////////////////////////////////////////////////
//
// TextLayout
//
TextLayout::TextLayout(IDWriteTextLayout* text_layout)
    : SimpleObject_(text_layout) {}

TextLayout::~TextLayout() {}

gfx::SizeF TextLayout::ComputeSize() const {
  DWRITE_TEXT_METRICS metrics;
  COM_VERIFY((*this)->GetMetrics(&metrics));
  return gfx::SizeF(metrics.width, metrics.height);
}

}  // namespace gfx
