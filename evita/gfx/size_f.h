// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_gfx_size_f_h)
#define INCLUDE_gfx_size_f_h

#include "base/logging.h"
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
    // TODO(eval1749) We should not allow negative |width| and |height|.
    //DCHECK_GE(width, static_cast<UnitType>(0));
    //DCHECK_GE(height, static_cast<UnitType>(0));
    this->width = width;
    this->height = height;
  }
  public: Size_(int width, int height) {
    // TODO(eval1749) We should not allow negative |width| and |height|.
    //DCHECK_GE(width, 0);
    //DCHECK_GE(height, 0);
    this->width = static_cast<UnitType>(width);
    this->height = static_cast<UnitType>(height);
  }

  public: bool operator==(const Size_& other) const;
  public: bool operator!=(const Size_& other) const;

  public: bool operator<(const Size_& other) const;
  public: bool operator<=(const Size_& other) const;
  public: bool operator>(const Size_& other) const;
  public: bool operator>=(const Size_& other) const;


  public: Size_ operator+(const Size_& other) const;
  public: Size_ operator+(UnitType scalar) const;
  public: Size_ operator-(const Size_& other) const;
  public: Size_ operator-(UnitType scalar) const;
  public: Size_ operator*(const Size_& other) const;
  public: Size_ operator*(UnitType scalar) const;
  public: Size_ operator/(const Size_& other) const;
  public: Size_ operator/(UnitType scalar) const;

  public: bool empty() const { return width <= 0 || height <= 0; }
  public: bool is_zero() const { return !width && !height; }

  public: UnitType GetArea() const { return width * height; }
};

template<typename BaseType, typename UnitType>
bool Size_<BaseType, UnitType>::operator==(const Size_& other) const {
  return width == other.width && height == other.height;
}

template<typename BaseType, typename UnitType>
bool Size_<BaseType, UnitType>::operator!=(const Size_& other) const {
  return !operator==(other);
}

template<typename BaseType, typename UnitType>
bool Size_<BaseType, UnitType>::operator<(const Size_& other) const {
  return GetArea() < other.GetArea();
}

template<typename BaseType, typename UnitType>
bool Size_<BaseType, UnitType>::operator<=(const Size_& other) const {
  return GetArea() <= other.GetArea();
}

template<typename BaseType, typename UnitType>
bool Size_<BaseType, UnitType>::operator>(const Size_& other) const {
  return GetArea() > other.GetArea();
}

template<typename BaseType, typename UnitType>
bool Size_<BaseType, UnitType>::operator>=(const Size_& other) const {
  return GetArea() >= other.GetArea();
}

template<typename BaseType, typename UnitType>
Size_<BaseType, UnitType> Size_<BaseType, UnitType>::operator+(
    const Size_& other) const {
  return Size_(width + other.width, height + other.height);
}

template<typename BaseType, typename UnitType>
Size_<BaseType, UnitType> Size_<BaseType, UnitType>::operator+(
    UnitType scalar) const {
  return Size_(width + scalar, height + scalar);
}

template<typename BaseType, typename UnitType>
Size_<BaseType, UnitType> Size_<BaseType, UnitType>::operator-(
    const Size_& other) const {
  return Size_(width - other.width, height - other.height);
}

template<typename BaseType, typename UnitType>
Size_<BaseType, UnitType> Size_<BaseType, UnitType>::operator-(
    UnitType scalar) const {
  return Size_(width - scalar, height - scalar);
}

template<typename BaseType, typename UnitType>
Size_<BaseType, UnitType> Size_<BaseType, UnitType>::operator*(
    const Size_& other) const {
  return Size_(width * other.width, height * other.height);
}

template<typename BaseType, typename UnitType>
Size_<BaseType, UnitType> Size_<BaseType, UnitType>::operator*(
    UnitType scalar) const {
  return Size_(width * scalar, height * scalar);
}

template<typename BaseType, typename UnitType>
Size_<BaseType, UnitType> Size_<BaseType, UnitType>::operator/(
    const Size_& other) const {
  return Size_(width / other.width, height / other.height);
}

template<typename BaseType, typename UnitType>
Size_<BaseType, UnitType> Size_<BaseType, UnitType>::operator/(
    UnitType scalar) const {
  return Size_(width / scalar, height / scalar);
}

// Common size types
typedef Size_<D2D1_SIZE_F, float> SizeF;
typedef Size_<D2D1_SIZE_U, uint32_t> SizeU;

} // namespace gfx

#define DEBUG_SIZEF_FORMAT "%dx%d"

#define DEBUG_SIZEF_ARG(size) \
  static_cast<int>((size).width * 1000), \
  static_cast<int>((size).height * 1000)

#include <ostream>
std::ostream& operator<<(std::ostream& ostream, const gfx::SizeF& size);
std::ostream& operator<<(std::ostream& ostream, const gfx::SizeF* size);

#endif //!defined(INCLUDE_gfx_size_f_h)
