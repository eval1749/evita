// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/gfx/text_format.h"

#include "evita/gfx/text_layout.h"

namespace gfx {

namespace {

static float pt2dip(int pt) {
  return static_cast<float>(pt) * 96.0f / 72.0f;
}

common::ComPtr<IDWriteTextFormat> CreateTextFormat(
    const base::string16 font_face_name,
    float font_size) {
  auto size = FactorySet::CeilToPixel(SizeF(0.0f, font_size));
  common::ComPtr<IDWriteTextFormat> text_format;
  COM_VERIFY(FactorySet::instance()->dwrite().CreateTextFormat(
      font_face_name.c_str(), nullptr, DWRITE_FONT_WEIGHT_REGULAR,
      DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, size.height,
      L"en-us", &text_format));
  return std::move(text_format);
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TextFormat
//
TextFormat::TextFormat(const base::string16& font_face_name, float font_size)
    : SimpleObject_(CreateTextFormat(font_face_name, font_size)) {}

TextFormat::TextFormat(const LOGFONT& log_font)
    : SimpleObject_(
          CreateTextFormat(log_font.lfFaceName, pt2dip(-log_font.lfHeight))) {}

std::unique_ptr<TextLayout> TextFormat::CreateLayout(const base::string16& text,
                                                     const SizeF& size) const {
  DCHECK(!size.empty());
  common::ComPtr<IDWriteTextLayout> text_layout;
  COM_VERIFY(FactorySet::instance()->dwrite().CreateTextLayout(
      text.data(), static_cast<UINT32>(text.length()), *this, size.width,
      size.height, &text_layout));
  if (!text_layout)
    return std::unique_ptr<TextLayout>();
  return std::make_unique<TextLayout>(text_layout);
}

float TextFormat::GetWidth(const base::string16& text) const {
  common::ComPtr<IDWriteTextLayout> text_layout;
  auto const kHuge = 1e6f;
  COM_VERIFY(FactorySet::instance()->dwrite().CreateTextLayout(
      text.data(), static_cast<UINT32>(text.length()), *this, kHuge, kHuge,
      &text_layout));
  auto width = 0.0f;
  COM_VERIFY(text_layout->DetermineMinWidth(&width));
  return width;
}

}  // namespace gfx
