// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/fonts/text_layout.h"

#include "evita/visuals/geometry/float_size.h"
#include "evita/visuals/fonts/text_format.h"

#if OS_WIN
#include "evita/visuals/fonts/native_text_layout_win.h"
#else
#error "Unsupported target"
#endif

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// TextLayout
//
TextLayout::TextLayout(const TextFormat& text_format,
                       const base::string16& text,
                       const FloatSize& size)
    : impl_(new NativeTextLayout(text_format.impl(), text, size)) {}

TextLayout::~TextLayout() {}

FloatSize TextLayout::GetMetrics() const {
  return impl_->GetMetrics();
}

}  // namespace visuals
