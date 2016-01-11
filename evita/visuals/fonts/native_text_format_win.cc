// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <dwrite.h>

#include "evita/visuals/fonts/native_text_format_win.h"

#include "common/win/com_verify.h"
#include "evita/visuals/fonts/direct_write_factory_win.h"
#include "evita/visuals/fonts/font_description.h"
#include "evita/visuals/fonts/native_text_layout_win.h"
#include "evita/visuals/geometry/float_size.h"

namespace visuals {

namespace {

DWRITE_FONT_STRETCH DirectWriteFontStretchOf(
    const FontDescription& description) {
  return static_cast<DWRITE_FONT_STRETCH>(description.stretch());
}

DWRITE_FONT_STYLE DirectWriteFontStyleOf(const FontDescription& description) {
  return static_cast<DWRITE_FONT_STYLE>(description.style());
}

DWRITE_FONT_WEIGHT DirectWriteFontWeightOf(const FontDescription& description) {
  return static_cast<DWRITE_FONT_WEIGHT>(description.weight());
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// NativeTextFormat
//
NativeTextFormat::NativeTextFormat(const FontDescription& description) {
  COM_VERIFY(DirectWriteFactory::GetInstance()->get()->CreateTextFormat(
      description.family().c_str(), nullptr,
      DirectWriteFontWeightOf(description), DirectWriteFontStyleOf(description),
      DirectWriteFontStretchOf(description), description.size(), L"en-us",
      text_format_.Receive()));
}

NativeTextFormat::~NativeTextFormat() {}

// Note: We should not use |IDWriteTextLayout::DetermineMinWidth()| here,
// since it returns width of word.
FloatSize NativeTextFormat::ComputeMetrics(const base::string16& text) const {
  base::win::ScopedComPtr<IDWriteTextLayout> text_layout;
  const auto kHuge = 1e6f;
  COM_VERIFY(DirectWriteFactory::GetInstance()->get()->CreateTextLayout(
      text.data(), static_cast<UINT32>(text.length()), text_format_.get(),
      kHuge, kHuge, text_layout.Receive()));
  DWRITE_TEXT_METRICS metrics;
  COM_VERIFY(text_layout->GetMetrics(&metrics));
  return FloatSize(metrics.width, metrics.height);
}

}  // namespace visuals
