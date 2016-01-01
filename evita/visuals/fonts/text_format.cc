// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/fonts/text_format.h"

#include "build/build_config.h"

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
TextFormat::TextFormat(const base::string16& font_face_name, float font_size)
    : impl_(new NativeTextFormat(font_face_name, font_size)) {}

TextFormat::~TextFormat() {}

float TextFormat::ComputeWidth(const base::string16& text) const {
  return impl_->ComputeWidth(text);
}

}  // namespace visuals
