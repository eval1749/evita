// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "evita/geometry/int_size.h"

namespace evita {

//////////////////////////////////////////////////////////////////////
//
// IntSize
//
IntSize::IntSize(int width, int height) : height_(height), width_(width) {}
IntSize::IntSize(const IntSize& other) : IntSize(other.width_, other.height_) {}
IntSize::IntSize() {}
IntSize::~IntSize() {}

IntSize& IntSize::operator=(const IntSize& other) {
  width_ = other.width_;
  height_ = other.height_;
  return *this;
}

bool IntSize::operator==(const IntSize& other) const {
  return width_ == other.width_ && height_ == other.height_;
}

bool IntSize::operator!=(const IntSize& other) const {
  return !operator==(other);
}

std::ostream& operator<<(std::ostream& ostream, const IntSize& size) {
  return ostream << size.width() << 'x' << size.height();
}

}  // namespace evita
