// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_GFX_RECT_F_H_
#define EVITA_GFX_RECT_F_H_

#include <algorithm>
#include <ostream>

#include "common/win/rect.h"
#include "evita/gfx/point_f.h"
#include "evita/gfx/size_f.h"

namespace gfx {

using common::win::Rect;

template <typename BaseType, typename PointType, typename SizeType>
class Rect_ final : public BaseType {
 public:
  typedef typename SizeType::UnitType UnitType;
  Rect_() { left = top = right = bottom = 0; }
  Rect_(UnitType left, UnitType top, UnitType right, UnitType bottom) {
    this->left = left;
    this->top = top;
    this->right = right;
    this->bottom = bottom;
  }
  Rect_(int left, int top, int right, int bottom) {
    this->left = static_cast<UnitType>(left);
    this->top = static_cast<UnitType>(top);
    this->right = static_cast<UnitType>(right);
    this->bottom = static_cast<UnitType>(bottom);
  }
  explicit Rect_(const RECT& other) = delete;
  explicit Rect_(const Rect& other) {
    this->left = static_cast<UnitType>(other.left());
    this->top = static_cast<UnitType>(other.top());
    this->right = static_cast<UnitType>(other.right());
    this->bottom = static_cast<UnitType>(other.bottom());
  }
  Rect_(const PointType& origin, const PointType& bottom_right) {
    left = origin.x;
    top = origin.y;
    right = bottom_right.x;
    bottom = bottom_right.y;
  }
  Rect_(const PointType& origin, const SizeType& size) {
    left = origin.x;
    top = origin.y;
    right = left + size.width;
    bottom = top + size.height;
  }
  explicit Rect_(const SizeType& size) {
    left = 0;
    top = 0;
    right = size.width;
    bottom = size.height;
  }

  operator RECT() const {
    RECT rc;
    rc.left = static_cast<long>(left);      // NOLINT
    rc.top = static_cast<long>(top);        // NOLINT
    rc.right = static_cast<long>(right);    // NOLINT
    rc.bottom = static_cast<long>(bottom);  // NOLINT
    return rc;
  }

  explicit operator bool() const { return !empty(); }
  bool operator!() const { return empty(); }

  Rect_& operator+=(const Rect_& other);

  Rect_ operator+(const Rect_& other) const { return Union(other); }
  Rect_ operator+(const SizeType& size) const;
  Rect_ operator-(const SizeType& size) const;

  Rect_ operator+(const UnitType& scalar) const {
    return operator+(SizeType(scalar, scalar));
  }

  Rect_ operator-(const UnitType& scalar) const {
    return operator-(SizeType(scalar, scalar));
  }

  Rect_ operator*(const SizeType& size) const {
    return Rect_(left * size.width, top * size.height, right * size.width,
                 bottom * size.height);
  }

  Rect_& operator*=(const SizeType& size) {
    left *= size.width;
    top *= size.height;
    right *= size.width;
    bottom *= size.height;
    return *this;
  }

  bool operator==(const Rect_& other) const {
    return left == other.left && top == other.top && right == other.right &&
           bottom == other.bottom;
  }

  bool operator!=(const Rect_& other) const {
    return left != other.left || top != other.top || right != other.right ||
           bottom != other.bottom;
  }

  bool operator<(const Rect_& other) const;
  bool operator<=(const Rect_& other) const;
  bool operator>(const Rect_& other) const;
  bool operator>=(const Rect_& other) const;

  UnitType area() const { return width() * height(); }
  PointType bottom_left() const { return PointType(left, bottom); }
  PointType bottom_right() const { return PointType(right, bottom); }
  UnitType height() const { return bottom - top; }

  // Return true if the area is zero or negative.
  bool empty() const { return width() <= 0 || height() <= 0; }

  // Returns true if the area of the rectangle is zero.
  bool is_zero() const { return !width() && !height(); }

  PointType origin() const { return PointType(left, top); }
  void set_origin(const PointType& origin) {
    auto const width = this->width();
    auto const height = this->height();
    left = origin.x;
    top = origin.y;
    right = left + width;
    bottom = top + height;
  }

  SizeType size() const { return SizeType(width(), height()); }
  void set_size(const SizeType& size) {
    right = left + size.width;
    bottom = top + size.height;
  }

  PointType top_right() const { return PointType(right, top); }

  UnitType width() const { return right - left; }

  // Returns true if the point identified by point_x and point_y falls inside
  // this rectangle.  The point (x, y) is inside the rectangle, but the
  // point (x + width, y + height) is not.
  bool Contains(PointF point) const;

  // Returns true if the specified point is contained by this rectangle.
  bool Contains(const Rect_& point) const;

  // Shrink the rectangle by a horizontal and vertical distance on all sides.
  Rect_ Inset(UnitType horizontal, UnitType vertical) const;

  // Move the rectangle by a horizontal and vertical distance.
  Rect_ Offset(UnitType horizontal, UnitType vertical) const;
  Rect_ Offset(const PointType& point) const;
  Rect_ Offset(const SizeType& size) const;

  // Returns true if this rectangle intersects the specified rectangle.
  // An empty rectangle doesn't intersect any rectangle.
  Rect_ Intersect(const Rect_& other) const;

  // Computes the union of this rectangle with the given rectangle.  The union
  // is the smallest rectangle containing both rectangles.
  Rect_ Union(const Rect_& other) const;
};

// Rect_ inline functions

template <typename BaseType, typename PointType, typename SizeType>
Rect_<BaseType, PointType, SizeType>& Rect_<BaseType, PointType, SizeType>::
operator+=(const Rect_& other) {
  if (other.empty())
    return *this;
  if (empty()) {
    *this = other;
    return *this;
  }
  left = std::min(left, other.left);
  top = std::min(top, other.top);
  right = std::max(right, other.right);
  bottom = std::max(bottom, other.bottom);
  return *this;
}

template <typename BaseType, typename PointType, typename SizeType>
Rect_<BaseType, PointType, SizeType> Rect_<BaseType, PointType, SizeType>::
operator+(const SizeType& size) const {
  return Rect_(left - size.width, top - size.height, right + size.width,
               bottom + size.height);
}

template <typename BaseType, typename PointType, typename SizeType>
Rect_<BaseType, PointType, SizeType> Rect_<BaseType, PointType, SizeType>::
operator-(const SizeType& size) const {
  return Rect_(left + size.width, top + size.height, right - size.width,
               bottom - size.height);
}

template <typename BaseType, typename PointType, typename SizeType>
bool Rect_<BaseType, PointType, SizeType>::operator<(const Rect_& other) const {
  return area() < other.area();
}

template <typename BaseType, typename PointType, typename SizeType>
bool Rect_<BaseType, PointType, SizeType>::operator<=(
    const Rect_& other) const {
  return area() <= other.area();
}

template <typename BaseType, typename PointType, typename SizeType>
bool Rect_<BaseType, PointType, SizeType>::operator>(const Rect_& other) const {
  return area() > other.area();
}

template <typename BaseType, typename PointType, typename SizeType>
bool Rect_<BaseType, PointType, SizeType>::operator>=(
    const Rect_& other) const {
  return area() >= other.area();
}

template <typename BaseType, typename PointType, typename SizeType>
bool Rect_<BaseType, PointType, SizeType>::Contains(PointF point) const {
  return point.x >= left && point.x < right && point.y >= top &&
         point.y < bottom;
}

template <typename BaseType, typename PointType, typename SizeType>
bool Rect_<BaseType, PointType, SizeType>::Contains(const Rect_& other) const {
  return left <= other.left && right >= other.right && top <= other.top &&
         bottom >= other.bottom;
}

template <typename BaseType, typename PointType, typename SizeType>
Rect_<BaseType, PointType, SizeType>
Rect_<BaseType, PointType, SizeType>::Inset(UnitType horizontal,
                                            UnitType vertical) const {
  return Rect_(left + horizontal, top + vertical, right - horizontal,
               bottom - vertical);
}

template <typename BaseType, typename PointType, typename SizeType>
Rect_<BaseType, PointType, SizeType>
Rect_<BaseType, PointType, SizeType>::Intersect(const Rect_& other) const {
  return Rect_(std::max(left, other.left), std::max(top, other.top),
               std::min(right, other.right), std::min(bottom, other.bottom));
}

template <typename BaseType, typename PointType, typename SizeType>
Rect_<BaseType, PointType, SizeType>
Rect_<BaseType, PointType, SizeType>::Offset(const PointType& point) const {
  return Rect_(left + point.x, top + point.y, right + point.x,
               bottom + point.y);
}

template <typename BaseType, typename PointType, typename SizeType>
Rect_<BaseType, PointType, SizeType>
Rect_<BaseType, PointType, SizeType>::Offset(const SizeType& size) const {
  return Rect_(left + size.width, top + size.height, right + size.width,
               bottom + size.height);
}

template <typename BaseType, typename PointType, typename SizeType>
Rect_<BaseType, PointType, SizeType>
Rect_<BaseType, PointType, SizeType>::Offset(UnitType width,
                                             UnitType height) const {
  return Offset(SizeType(width, height));
}

template <typename BaseType, typename PointType, typename SizeType>
Rect_<BaseType, PointType, SizeType>
Rect_<BaseType, PointType, SizeType>::Union(const Rect_& other) const {
  if (other.empty())
    return *this;
  if (empty())
    return other;
  return gfx::RectF(
      gfx::PointF(std::min(left, other.left), std::min(top, other.top)),
      gfx::PointF(std::max(right, other.right),
                  std::max(bottom, other.bottom)));
}

typedef Rect_<D2D1_RECT_F, PointF, SizeF> RectF;
typedef Rect_<D2D1_RECT_U, PointU, SizeU> RectU;

}  // namespace gfx

namespace std {
template <>
struct hash<gfx::RectF> {
  size_t operator()(const gfx::RectF& rect) const;
};
}  // namespace std

#define DEBUG_RECTF_FORMAT "(%d,%d)+%dx%d"

#define DEBUG_RECTF_ARG(rect)                                                \
  static_cast<int>((rect).left * 1000), static_cast<int>((rect).top * 1000), \
      static_cast<int>((rect).width() * 1000),                               \
      static_cast<int>((rect).height() * 1000)

std::ostream& operator<<(std::ostream& ostream, const gfx::RectF& rect);
std::ostream& operator<<(std::ostream& ostream, const gfx::RectF* rect);

#endif  // EVITA_GFX_RECT_F_H_
