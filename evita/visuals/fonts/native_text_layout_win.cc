// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <dwrite.h>
#include <stdint.h>

#include "evita/visuals/fonts/native_text_layout_win.h"

#include "common/win/com_verify.h"
#include "evita/visuals/fonts/direct_write_factory_win.h"
#include "evita/visuals/fonts/native_text_format_win.h"
#include "evita/visuals/geometry/float_size.h"

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

}  // namespace visuals
