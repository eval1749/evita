// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_GFX_POINT_F_H_
#define EVITA_GFX_POINT_F_H_

#include <ostream>

#include "evita/gfx/size_f.h"

namespace gfx {

template <typename BaseType, typename SizeType>
class Point_ final : public BaseType {
 public:
  typedef typename SizeType::UnitType UnitType;
  Point_(const Point_& other) {
    x = other.x;
    y = other.y;
  }
  Point_(UnitType x, UnitType y) {
    this->x = x;
    this->y = y;
  }
  Point_(int x, int y) {
    this->x = static_cast<UnitType>(x);
    this->y = static_cast<UnitType>(y);
  }
  explicit Point_(const POINT& point) {
    x = static_cast<UnitType>(point.x);
    y = static_cast<UnitType>(point.y);
  }
  Point_() {
    this->x = static_cast<UnitType>(0);
    this->y = static_cast<UnitType>(0);
  }

  Point_& operator=(const Point_& other) {
    x = other.x;
    y = other.y;
    return *this;
  }

  Point_& operator+=(const Point_& other) {
    x += other.x;
    y += other.y;
    return *this;
  }

  Point_& operator+=(const SizeType& other) {
    x += other.width;
    y += other.height;
    return *this;
  }

  Point_& operator-=(const Point_& other) {
    x -= other.x y -= other.y;
    return *this;
  }

  Point_& operator-=(const SizeType& other) {
    x -= other.width y -= other.height;
    return *this;
  }

  bool operator==(const Point_& other) const {
    return x == other.x && y == other.y;
  }

  bool operator!=(const Point_& other) const { return !operator==(other); }

  Point_ operator+(const Point_& other) const {
    return Point_(x + other.x, y + other.y);
  }

  Point_ operator+(const SizeType& size) const {
    return Point_(x + size.width, y + size.height);
  }

  Point_ operator-(const Point_& other) const {
    return Point_(x - other.x, y - other.y);
  }

  Point_ operator-(const SizeType& size) const {
    return Point_(x - size.width, y - size.height);
  }

  Point_ operator*(const Point_& other) const {
    return Point_(x * other.x, y * other.y);
  }

  Point_ operator*(const SizeType& size) const {
    return Point_(x * size.width, y * size.height);
  }

  Point_ operator/(UnitType divisor) const {
    return Point_(x / divisor, y / divisor);
  }

  Point_ Offset(UnitType delta_x, UnitType delta_y) const {
    return Point_(x + delta_x, y + delta_y);
  }
};
typedef Point_<D2D1_POINT_2F, SizeF> PointF;
typedef Point_<D2D1_POINT_2U, SizeU> PointU;

}  // namespace gfx

#define DEBUG_POINTF_FORMAT "(%d,%d)"

#define DEBUG_POINTF_ARG(point) \
  static_cast<int>((point).x * 1000), static_cast<int>((point).y * 1000)

std::ostream& operator<<(std::ostream& ostream, const gfx::PointF& point);
std::ostream& operator<<(std::ostream& ostream, const gfx::PointF* point);

#endif  // EVITA_GFX_POINT_F_H_
