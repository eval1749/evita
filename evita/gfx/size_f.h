// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_gfx_size_f_h)
#define INCLUDE_gfx_size_f_h

#include <stdint.h>

#include "evita/gfx/forward.h"

namespace gfx {

template<typename BaseType, typename UnitType>
class Size_ : public BaseType {
  public: typedef UnitType UnitType;

  public: Size_() {
    width = height = 0;
  }
  public: Size_(const BaseType& other) {
    width = other.width;
    height = other.height;
  }
  public: Size_(UnitType width, UnitType height) {
    this->width = width;
    this->height = height;
  }
  public: Size_(int width, int height) {
    this->width = static_cast<UnitType>(width);
    this->height = static_cast<UnitType>(height);
  }
  public: bool operator==(const Size_& other) const {
    return width == other.width && height == other.height;
  }
  public: bool operator!=(const Size_& other) const {
    return width != other.width && height != other.height;
  }

  public: Size_ operator+(const Size_& other) const {
    return Size_(width + other.width, height + other.height);
  }

  public: Size_ operator+(UnitType scalar) const {
    return Size_(width + scalar, height + scalar);
  }

  public: Size_ operator-(const Size_& other) const {
    return Size_(width - other.width, height - other.height);
  }

  public: Size_ operator-(UnitType scalar) const {
    return Size_(width - scalar, height - scalar);
  }

  public: Size_ operator*(const Size_& other) const {
    return Size_(width * other.width, height * other.height);
  }

  public: Size_ operator*(UnitType scalar) const {
    return Size_(width * scalar, height * scalar);
  }

  public: Size_ operator/(const Size_& other) const {
    return Size_(width / other.width, height / other.height);
  }

  public: Size_ operator/(UnitType scalar) const {
    return Size_(width / scalar, height / scalar);
  }

  public: bool is_empty() const { return width <= 0 || height <= 0; }
  public: bool is_zero() const { return !width && !height; }
};

typedef Size_<D2D1_SIZE_F, float> SizeF;
typedef Size_<D2D1_SIZE_U, uint32_t> SizeU;

} // namespace gfx

#define DEBUG_SIZEF_FORMAT "%dx%d"

#define DEBUG_SIZEF_ARG(size) \
  static_cast<int>((size).width * 1000), \
  static_cast<int>((size).height * 1000)

#endif //!defined(INCLUDE_gfx_size_f_h)
