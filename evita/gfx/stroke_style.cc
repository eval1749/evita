// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/gfx/stroke_style.h"

namespace gfx {

StrokeStyle::StrokeStyle(const StrokeStyle& other)
    : platform_style_(other.platform_style_) {}

StrokeStyle::StrokeStyle(StrokeStyle&& other)
    : platform_style_(std::move(other.platform_style_)) {}

StrokeStyle::StrokeStyle(common::ComPtr<ID2D1StrokeStyle1>&& platform_style)
    : platform_style_(std::move(platform_style)) {}

StrokeStyle::~StrokeStyle() = default;

StrokeStyle::operator ID2D1StrokeStyle*() const {
  return platform_style_.get();
}

StrokeStyle& StrokeStyle::operator=(const StrokeStyle& other) {
  platform_style_ = other.platform_style_;
  return *this;
}

StrokeStyle& StrokeStyle::operator=(StrokeStyle&& other) {
  platform_style_ = std::move(other.platform_style_);
  return *this;
}

}  // namespace gfx
