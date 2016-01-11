// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "evita/visuals/fonts/text_format.h"

#include "build/build_config.h"
#include "evita/visuals/fonts/font_description.h"
#include "evita/visuals/geometry/float_size.h"

#if OS_WIN
#include "evita/visuals/fonts/native_text_format_win.h"
#else
#error "Unsupported target"
#endif

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// TextFormat
//
TextFormat::TextFormat(const FontDescription& description)
    : font_description_(description),
      impl_(new NativeTextFormat(description)) {}

TextFormat::~TextFormat() {}

FloatSize TextFormat::ComputeMetrics(const base::string16& text) const {
  return impl_->ComputeMetrics(text);
}

std::ostream& operator<<(std::ostream& ostream, const TextFormat& text_format) {
  return ostream << "TextFormat(" << text_format.font_description() << ')';
}

}  // namespace visuals
