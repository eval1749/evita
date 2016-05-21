// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "evita/gfx/base/geometry/float_size.h"

#include "base/logging.h"

namespace gfx {

//////////////////////////////////////////////////////////////////////
//
// FloatSize
//
FloatSize::FloatSize(const std::pair<float, float> pair)
    : FloatSize(pair.first, pair.second) {}

FloatSize::FloatSize(float size) : FloatSize(size, size) {}

FloatSize::FloatSize(float width, float height)
    : height_(height), width_(width) {
  DCHECK_GE(height_, 0.0f);
  DCHECK_GE(width_, 0.0f);
}

FloatSize::FloatSize(const FloatSize& other)
    : FloatSize(other.width_, other.height_) {}

FloatSize::FloatSize() {}
FloatSize::~FloatSize() {}

FloatSize& FloatSize::operator=(const FloatSize& other) {
  width_ = other.width_;
  height_ = other.height_;
  return *this;
}

bool FloatSize::operator==(const FloatSize& other) const {
  return width_ == other.width_ && height_ == other.height_;
}

bool FloatSize::operator!=(const FloatSize& other) const {
  return !operator==(other);
}

FloatSize FloatSize::operator+(const FloatSize& size) const {
  return FloatSize(width_ + size.width(), height_ + size.height());
}

FloatSize FloatSize::operator-(const FloatSize& size) const {
  return FloatSize(width_ - size.width(), height_ - size.height());
}

std::ostream& operator<<(std::ostream& ostream, const FloatSize& size) {
  return ostream << size.width() << 'x' << size.height();
}

}  // namespace gfx
