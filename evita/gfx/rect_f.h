// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_gfx_rect_f_h)
#define INCLUDE_gfx_rect_f_h

#include <algorithm>

#include "gfx/point_f.h"
#include "gfx/size_f.h"

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
  public: Rect_(const PointType& left_top, const PointType& right_bottom) {
    left = left_top.x;
    top = left_top.y;
    right = right_bottom.x;
    bottom = right_bottom.y;
  }
  public: Rect_(const PointType& left_top, const SizeType& size) {
    left = left_top.x;
    top = left_top.y;
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

  public: Rect_ operator*(const SizeType& size) const {
    return Rect_(left * size.width, top * size.height,
                 right * size.width, bottom * size.height);
  }

  public: operator bool() const { return !is_empty(); }
  public: bool operator!() const { return is_empty(); }

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

  public: UnitType height() const { return bottom - top; }

  public: bool is_empty() const {
    return width() <= 0 || height() <= 0;
  }

  public: bool is_zero() const { return !width() && !height(); }

  public: PointType left_top() const {
    return PointType(left, top);
  }

  public: PointType right_bottom() const {
    return PointType(right, bottom);
  }

  public: SizeType size() const { return SizeType(width(), height()); }
  public: UnitType width() const { return right - left; }

  public: bool Contains(PointF point) const;
  public: void Unite(const Rect_& other);
};

template<typename BaseType, typename PointType, typename SizeType>
bool Rect_<BaseType, PointType, SizeType>::Contains(PointF point) const {
  return point.x >= left && point.x < right && point.y >= top &&
         point.y < bottom;
}

template<typename BaseType, typename PointType, typename SizeType>
void Rect_<BaseType, PointType, SizeType>::Unite(const Rect_& other) {
  if (other.is_empty())
    return;
  if (is_empty()) {
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

#define DEBUG_RECTF_FORMAT "(%d,%d)+%dx%d"

#define DEBUG_RECTF_ARG(rect) \
    static_cast<int>((rect).left * 1000), \
    static_cast<int>((rect).top * 1000), \
    static_cast<int>((rect).width() * 1000), \
    static_cast<int>((rect).height() * 1000)

#endif //!defined(INCLUDE_gfx_rect_f_h)
