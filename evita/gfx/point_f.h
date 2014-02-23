// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_gfx_point_f_h)
#define INCLUDE_gfx_point_f_h

#include "evita/gfx/size_f.h"

namespace gfx {

template<typename BaseType, typename SizeType>
class Point_ : public BaseType {
  public: typedef typename SizeType::UnitType UnitType;
  public: Point_(const Point_& other) {
    x = other.x;
    y = other.y;
  }
  public: Point_(UnitType x, UnitType y) {
    this->x = x;
    this->y = y;
  }
  public: Point_(int x, int y) {
    this->x = static_cast<UnitType>(x);
    this->y = static_cast<UnitType>(y);
  }
  public: Point_(const POINT& point) {
    x = static_cast<UnitType>(point.x);
    y = static_cast<UnitType>(point.y);
  }
  public: Point_() {
    this->x = static_cast<UnitType>(0);
    this->y = static_cast<UnitType>(0);
  }

  public: Point_& operator=(const Point_& other) {
    x = other.x;
    y = other.y;
    return *this;
  }

  public: Point_ operator+(const Point_& other) const {
    return Point_(x + other.x, y + other.y);
  }

  public: Point_ operator+(const SizeType& size) const {
    return Point_(x + size.width, y + size.height);
  }

  public: Point_ operator-(const Point_& other) const {
    return Point_(x - other.x, y - other.y);
  }

  public: Point_ operator-(const SizeType& size) const {
    return Point_(x - size.width, y - size.height);
  }

  public: Point_ operator*(const Point_& other) const {
    return Point_(x * other.x, y * other.y);
  }

  public: Point_ operator*(const SizeType& size) const {
    return Point_(x * size.width, y * size.height);
  }

  public: Point_ operator/(UnitType divisor) const {
    return Point_(x / divisor, y / divisor);
  }
};
typedef Point_<D2D1_POINT_2F, SizeF> PointF;
typedef Point_<D2D1_POINT_2U, SizeU> PointU;

} // namespace gfx

#define DEBUG_POINTF_FORMAT "(%d,%d)"

#define DEBUG_POINTF_ARG(point) \
  static_cast<int>((point).x * 1000), \
  static_cast<int>((point).y * 1000)

#include <ostream>
std::ostream& operator<<(std::ostream& ostream, const gfx::PointF& point);
std::ostream& operator<<(std::ostream& ostream, const gfx::PointF* point);

#endif //!defined(INCLUDE_gfx_pont_f_h)
