// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common/win/size.h"

#include "base/logging.h"

namespace common {
namespace win {

Size::Size(const Size& other) : data_(other.data_) {}

Size::Size(int width, int height) {
  DCHECK_GE(width, 0);
  DCHECK_GE(height, 0);
  data_.cx = width;
  data_.cy = height;
}

Size::Size() : Size(0, 0) {}

Size& Size::operator=(const Size& other) {
  data_ = other.data_;
  return *this;
}

bool Size::operator==(const Size& other) const {
  return width() == other.width() && height() == other.height();
}

bool Size::operator!=(const Size& other) const {
  return !operator==(other);
}

bool Size::operator<(const Size& other) const {
  return GetArea() < other.GetArea();
}

bool Size::operator<=(const Size& other) const {
  return GetArea() < other.GetArea();
}

bool Size::operator>(const Size& other) const {
  return GetArea() > other.GetArea();
}

bool Size::operator>=(const Size& other) const {
  return GetArea() > other.GetArea();
}

Size Size::operator+(const Size& other) const {
  return Size(width() + other.width(), height() + other.height());
}

Size Size::operator-(const Size& other) const {
  return Size(width() - other.width(), height() - other.height());
}

Size Size::operator*(int multiplier) const {
  return Size(width() * multiplier, height() * multiplier);
}

Size Size::operator/(int divider) const {
  return Size(width() / divider, height() / divider);
}

int Size::GetArea() const {
  return width() * height();
}

}  // namespace win
}  // namespace common

COMMON_EXPORT std::ostream& operator<<(std::ostream& out,
                                       const common::win::Size& size) {
  return out << size.width() << "x" << size.height();
}

COMMON_EXPORT std::ostream& operator<<(std::ostream& out,
                                       const common::win::Size* size) {
  return out << *size;
}
