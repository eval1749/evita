// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_gfx_rect_f_h)
#define INCLUDE_gfx_rect_f_h

#include <algorithm>

#include "evita/gfx/point_f.h"
#include "evita/gfx/size_f.h"

namespace gfx {

template<typename BaseType, typename PointType, typename SizeType>
class Rect_ : public BaseType {
  public: typedef typename SizeType::UnitType UnitType;
  public: Rect_() {
    left = top = right = bottom = 0;
  }
  public: Rect_(UnitType left, UnitType top, UnitType right, UnitType bottom) {
    this->left = left;
    this->top = top;
    this->right = right;
    this->bottom = bottom;
  }
  public: Rect_(int left, int top, int right, int bottom) {
    this->left = static_cast<UnitType>(left);
    this->top = static_cast<UnitType>(top);
    this->right = static_cast<UnitType>(right);
    this->bottom = static_cast<UnitType>(bottom);
  }
  public: explicit Rect_(const RECT& rc) {
    this->left = static_cast<UnitType>(rc.left);
    this->top = static_cast<UnitType>(rc.top);
    this->right = static_cast<UnitType>(rc.right);
    this->bottom = static_cast<UnitType>(rc.bottom);
  }
  public: Rect_(const PointType& origin, const PointType& bottom_right) {
    left = origin.x;
    top = origin.y;
    right = bottom_right.x;
    bottom = bottom_right.y;
  }
  public: Rect_(const PointType& origin, const SizeType& size) {
    left = origin.x;
    top = origin.y;
    right = left + size.width;
    bottom = top + size.height;
  }
  public: explicit Rect_(const SizeType& size) {
    left = 0;
    top = 0;
    right = size.width;
    bottom = size.height;
  }

  public: operator RECT() const {
    RECT rc;
    rc.left = static_cast<long>(left);
    rc.top = static_cast<long>(top);
    rc.right = static_cast<long>(right);
    rc.bottom = static_cast<long>(bottom);
    return rc;
  }

  public: explicit operator bool() const { return !empty(); }
  public: bool operator!() const { return empty(); }

  public: Rect_ operator+(const SizeType& size) const;
  public: Rect_ operator-(const SizeType& size) const;

  public: Rect_ operator+(const UnitType& scalar) const {
    return operator+(SizeType(scalar, scalar));
  }

  public: Rect_ operator-(const UnitType& scalar) const {
    return operator-(SizeType(scalar, scalar));
  }

  public: Rect_ operator*(const SizeType& size) const {
    return Rect_(left * size.width, top * size.height,
                 right * size.width, bottom * size.height);
  }

  public: Rect_& operator*=(const SizeType& size) {
    left *= size.width;
    top *= size.height;
    right *= size.width;
    bottom *= size.height;
    return *this;
  }

  public: bool operator==(const Rect_& other) const {
    return left == other.left && top == other.top &&
           right == other.right && bottom == other.bottom;
  }

  public: bool operator!=(const Rect_& other) const {
    return left != other.left || top != other.top ||
           right != other.right || bottom != other.bottom;
  }

  public: bool operator<(const Rect_& other) const;
  public: bool operator<=(const Rect_& other) const;
  public: bool operator>(const Rect_& other) const;
  public: bool operator>=(const Rect_& other) const;

  public: UnitType area() const { return width() * hieght(); }
  public: PointType bottom_right() const {
    return PointType(right, bottom);
  }
  public: UnitType height() const { return bottom - top; }

  public: bool empty() const {
    return width() <= 0 || height() <= 0;
  }

  public: bool is_zero() const { return !width() && !height(); }

  public: PointType origin() const {
    return PointType(left, top);
  }
  public: void set_origin(const PointType& origin) {
    auto const width = this->width();
    auto const height = this->height();
    left = origin.x;
    top = origin.y;
    right = left + width;
    bottom = top + height;
  }

  public: SizeType size() const { return SizeType(width(), height()); }
  public: void set_size(const SizeType& size) {
    right = left + size.width;
    bottom = top + size.height;
  }

  public: UnitType width() const { return right - left; }

  public: bool Contains(PointF point) const;
  public: bool Contains(const Rect_& point) const;
  public: Rect_ Offset(UnitType offset_x, UnitType offset_y) const;
  public: Rect_ Offset(const PointType& size) const;
  public: Rect_ Offset(const SizeType& size) const;
  public: Rect_ Intersect(const Rect_& other) const;
  public: void Unite(const Rect_& other);
};

// Rect_ inline functions

template<typename BaseType, typename PointType, typename SizeType>
Rect_<BaseType, PointType, SizeType>
    Rect_<BaseType, PointType, SizeType>::operator+(
        const SizeType& size) const {
  return Rect_(left - size.width, top - size.height,
               right + size.width, bottom + size.height);
}

template<typename BaseType, typename PointType, typename SizeType>
Rect_<BaseType, PointType, SizeType>
    Rect_<BaseType, PointType, SizeType>::operator-(
        const SizeType& size) const {
  return Rect_(left + size.width, top + size.height,
               right - size.width, bottom - size.height);
}

template<typename BaseType, typename PointType, typename SizeType>
bool Rect_<BaseType, PointType, SizeType>::operator<(
    const Rect_& other) const {
  return area() < other.area();
}

template<typename BaseType, typename PointType, typename SizeType>
bool Rect_<BaseType, PointType, SizeType>::operator<=(
    const Rect_& other) const {
  return area() <= other.area();
}

template<typename BaseType, typename PointType, typename SizeType>
bool Rect_<BaseType, PointType, SizeType>::operator>(
    const Rect_& other) const {
  return area() > other.area();
}

template<typename BaseType, typename PointType, typename SizeType>
bool Rect_<BaseType, PointType, SizeType>::operator>=(
    const Rect_& other) const {
  return area() >= other.area();
}

template<typename BaseType, typename PointType, typename SizeType>
bool Rect_<BaseType, PointType, SizeType>::Contains(PointF point) const {
  return point.x >= left && point.x < right && point.y >= top &&
         point.y < bottom;
}

template<typename BaseType, typename PointType, typename SizeType>
bool Rect_<BaseType, PointType, SizeType>::Contains(const Rect_& other) const {
  return left <= other.left && right >= other.right &&
         top <= other.top && bottom >= other.bottom;
}

template<typename BaseType, typename PointType, typename SizeType>
Rect_<BaseType, PointType, SizeType>
Rect_<BaseType, PointType, SizeType>::Intersect(const Rect_& other) const {
  return Rect_(std::max(left, other.left),
                    std::max(top, other.top),
                    std::min(right, other.right),
                    std::min(bottom, other.bottom));
}

template<typename BaseType, typename PointType, typename SizeType>
Rect_<BaseType, PointType, SizeType>
Rect_<BaseType, PointType, SizeType>::Offset(const PointType& point) const {
  return Rect_(left + point.x, top + point.y,
               right + point.x, bottom + point.y);
}

template<typename BaseType, typename PointType, typename SizeType>
Rect_<BaseType, PointType, SizeType>
Rect_<BaseType, PointType, SizeType>::Offset(const SizeType& size) const {
  return Rect_(left + size.width, top + size.height,
               right + size.width, bottom + size.height);
}

template<typename BaseType, typename PointType, typename SizeType>
Rect_<BaseType, PointType, SizeType>
Rect_<BaseType, PointType, SizeType>::Offset(
    UnitType width, UnitType height) const {
  return Offset(SizeType(width, height));
}

template<typename BaseType, typename PointType, typename SizeType>
void Rect_<BaseType, PointType, SizeType>::Unite(const Rect_& other) {
  if (other.empty())
    return;
  if (empty()) {
    *this = other;
    return;
  }
  left = std::min(left, other.left);
  top = std::min(top, other.top);
  right = std::max(right, other.right);
  bottom = std::max(bottom, other.bottom);
}

typedef Rect_<D2D1_RECT_F, PointF, SizeF> RectF;
typedef Rect_<D2D1_RECT_U, PointU, SizeU> RectU;

} // namespace gfx

namespace std {
template<> struct hash<gfx::RectF> {
  size_t operator()(const gfx::RectF& rect) const;
};
}  // namespace std

#define DEBUG_RECTF_FORMAT "(%d,%d)+%dx%d"

#define DEBUG_RECTF_ARG(rect) \
    static_cast<int>((rect).left * 1000), \
    static_cast<int>((rect).top * 1000), \
    static_cast<int>((rect).width() * 1000), \
    static_cast<int>((rect).height() * 1000)

#include <ostream>
std::ostream& operator<<(std::ostream& ostream, const gfx::RectF& rect);
std::ostream& operator<<(std::ostream& ostream, const gfx::RectF* rect);

#endif //!defined(INCLUDE_gfx_rect_f_h)
