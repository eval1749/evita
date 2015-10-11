// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include <algorithm>

#include "common/win/rect.h"

#include "base/logging.h"

namespace common {
namespace win {

Rect::Rect(const Point& origin, const Point& bottom_right)
    : Rect(origin.x(), origin.y(), bottom_right.x(), bottom_right.y()) {}

Rect::Rect(const Point& origin, const Size& size)
    : Rect(origin, origin + size) {}

Rect::Rect(const Size& size) : Rect(0, 0, size.width(), size.height()) {}

Rect::Rect(int left, int top, int right, int bottom) {
  DCHECK_LE(left, right);
  DCHECK_LE(top, bottom);
  data_.left = left;
  data_.right = right;
  data_.top = top;
  data_.bottom = bottom;
}

Rect::Rect(const Rect& other) : data_(other.data_) {}

Rect::Rect(const RECT& other)
    : Rect(other.left, other.top, other.right, other.bottom) {}

Rect::Rect() : Rect(0, 0, 0, 0) {}

Rect::~Rect() {}

Rect::operator RECT() const {
  RECT rect;
  rect.left = left();
  rect.top = top();
  rect.right = right();
  rect.bottom = bottom();
  return rect;
}

Rect& Rect::operator=(const Rect& other) {
  data_ = other.data_;
  return *this;
}

Rect& Rect::operator+=(const Rect& other) {
  if (other.empty())
    return *this;
  if (empty()) {
    *this = other;
    return *this;
  }
  data_.left = std::min(left(), other.left());
  data_.top = std::min(top(), other.top());
  data_.right = std::max(right(), other.right());
  data_.bottom = std::max(bottom(), other.bottom());
  return *this;
}

bool Rect::operator==(const Rect& other) const {
  return left() == other.left() && top() == other.top() &&
         right() == other.right() && bottom() == other.bottom();
}

bool Rect::operator!=(const Rect& other) const {
  return !operator==(other);
}

bool Rect::operator<(const Rect& other) const {
  return area() < other.area();
}

bool Rect::operator<=(const Rect& other) const {
  return area() <= other.area();
}

bool Rect::operator>(const Rect& other) const {
  return area() > other.area();
}

bool Rect::operator>=(const Rect& other) const {
  return area() >= other.area();
}

Rect Rect::operator-(const Size& size) const {
  return Rect(left() + size.width(), top() + size.height(),
              right() - size.width(), bottom() - size.height());
}

inline void Rect::set_origin(const Point& new_origin) {
  auto const width = this->width();
  auto const height = this->height();
  data_.left = new_origin.x();
  data_.top = new_origin.y();
  data_.right = data_.left + width;
  data_.bottom = data_.top + height;
}

bool Rect::Contains(const Point& point) const {
  return point.x() >= left() && point.x() < right() && point.y() >= top() &&
         point.y() < bottom();
}

bool Rect::Contains(const Rect& other) const {
  return left() <= other.left() && right() >= other.right() &&
         top() <= other.top() && bottom() >= other.bottom();
}

Rect Rect::Intersect(const Rect& other) const {
  return Rect(std::max(left(), other.left()), std::max(top(), other.top()),
              std::min(right(), other.right()),
              std::min(bottom(), other.bottom()));
}

Rect Rect::Union(const Rect& other) const {
  if (other.empty())
    return *this;
  if (empty())
    return other;
  return Rect(std::min(left(), other.left()), std::min(top(), other.top()),
              std::max(right(), other.right()),
              std::max(bottom(), other.bottom()));
}

}  // namespace win
}  // namespace common

COMMON_EXPORT std::ostream& operator<<(std::ostream& out,
                                       const common::win::Rect& rect) {
  return out << rect.origin() << "+" << rect.size();
}

COMMON_EXPORT std::ostream& operator<<(std::ostream& out,
                                       const common::win::Rect* rect) {
  return out << *rect;
}
