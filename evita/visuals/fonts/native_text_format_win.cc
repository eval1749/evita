// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <dwrite.h>

#include "evita/visuals/fonts/native_text_format_win.h"

#include "common/win/com_verify.h"
#include "evita/visuals/fonts/direct_write_factory_win.h"
#include "evita/visuals/fonts/native_text_layout_win.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// NativeTextFormat
//
NativeTextFormat::NativeTextFormat(const base::string16& font_face_name,
                                   float font_size) {
  COM_VERIFY(DirectWriteFactory::GetInstance()->get()->CreateTextFormat(
      font_face_name.c_str(), nullptr, DWRITE_FONT_WEIGHT_REGULAR,
      DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, font_size, L"en-us",
      text_format_.Receive()));
}

NativeTextFormat::~NativeTextFormat() {}

float NativeTextFormat::ComputeWidth(const base::string16& text) const {
  base::win::ScopedComPtr<IDWriteTextLayout> text_layout;
  const auto kHuge = 1e6f;
  COM_VERIFY(DirectWriteFactory::GetInstance()->get()->CreateTextLayout(
      text.data(), static_cast<UINT32>(text.length()), text_format_.get(),
      kHuge, kHuge, text_layout.Receive()));
  auto width = 0.0f;
  COM_VERIFY(text_layout->DetermineMinWidth(&width));
  return width;
}

}  // namespace visuals
