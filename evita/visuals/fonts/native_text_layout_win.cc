// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <dwrite.h>
#include <stdint.h>

#include <cmath>

#include "evita/visuals/fonts/native_text_layout_win.h"

#include "common/win/com_verify.h"
#include "evita/visuals/fonts/direct_write_factory_win.h"
#include "evita/visuals/fonts/native_text_format_win.h"
#include "evita/visuals/geometry/float_rect.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// NativeTextLayout
//
NativeTextLayout::NativeTextLayout(const NativeTextFormat& text_format,
                                   const base::string16& text,
                                   const FloatSize& size) {
  DCHECK(!size.IsEmpty());
  COM_VERIFY(DirectWriteFactory::GetInstance()->get()->CreateTextLayout(
      text.data(), static_cast<uint32_t>(text.length()),
      text_format.get().get(), size.width(), size.height(), value_.Receive()));
}

NativeTextLayout::NativeTextLayout(const NativeTextLayout& other)
    : value_(other.value_) {}

NativeTextLayout::NativeTextLayout(NativeTextLayout&& other)
    : value_(std::move(other.value_)) {}

NativeTextLayout::~NativeTextLayout() {}

bool NativeTextLayout::operator==(const NativeTextLayout& other) const {
  return value_ == other.value_;
}

bool NativeTextLayout::operator!=(const NativeTextLayout& other) const {
  return !operator==(other);
}

FloatSize NativeTextLayout::GetMetrics() const {
  DWRITE_TEXT_METRICS metrics;
  COM_VERIFY(value_->GetMetrics(&metrics));
  return FloatSize(metrics.width, metrics.height);
}

size_t NativeTextLayout::HitTestPoint(const FloatPoint& point) const {
  BOOL is_inside = false;
  BOOL is_trailing = false;
  DWRITE_HIT_TEST_METRICS metrics = {0};
  COM_VERIFY(value_->HitTestPoint(point.x(), point.y(), &is_trailing,
                                  &is_inside, &metrics));
  if (!metrics.isText)
    return static_cast<size_t>(-1);
  return static_cast<size_t>(metrics.textPosition + is_trailing);
}

FloatRect NativeTextLayout::HitTestTextPosition(size_t offset) const {
  auto caret_x = 0.0f;
  auto caret_y = 0.0f;
  auto const is_trailing = false;
  DWRITE_HIT_TEST_METRICS metrics = {0};
  COM_VERIFY(value_->HitTestTextPosition(static_cast<uint32_t>(offset),
                                         is_trailing, &caret_x, &caret_y,
                                         &metrics));
  return FloatRect(
      FloatPoint(std::round(caret_x), std::round(caret_y)),
      FloatSize(std::round(metrics.width), std::round(metrics.height)));
}

}  // namespace visuals
