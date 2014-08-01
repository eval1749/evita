// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_common_win_size_h)
#define INCLUDE_common_win_size_h

#include "common/common_export.h"

namespace common {
namespace win {

class COMMON_EXPORT Size {
  private: SIZE data_;

  public: Size(const Size& other);
  public: Size(int width, int height);
  public: Size();

  public: explicit operator bool() const { return !empty(); }
  public: const Size* operator&() const = delete;
  public: Size* operator&() = delete;
  public: Size& operator=(const Size& other);

  public: bool operator==(const Size& other) const;
  public: bool operator!=(const Size& other) const;
  public: Size operator+(const Size& other) const;
  public: Size operator-(const Size& other) const;
  public: Size operator*(int multiplier) const;
  public: Size operator/(int divider) const;

  public: bool empty() const { return width() <= 0 || height() <= 0; }
  public: int height() const { return data_.cy; }
  public: void set_height(int height) { data_.cy = height; }
  public: SIZE* ptr();
  public: int width() const { return data_.cx; }
  public: void set_width(int width) { data_.cx = width; }
};

} // namespace win
} // namespace common

#include <ostream>

COMMON_EXPORT std::ostream& operator<<(std::ostream& out,
                                       const common::win::Size& size);
COMMON_EXPORT std::ostream& operator<<(std::ostream& out,
                                       const common::win::Size* size);

#endif //!defined(INCLUDE_common_win_size_h)
