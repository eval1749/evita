// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef COMMON_WIN_SIZE_H_
#define COMMON_WIN_SIZE_H_

#include <windows.h>

#include <ostream>

#include "common/common_export.h"

namespace common {
namespace win {

class COMMON_EXPORT Size {
 public:
  Size(const Size& other);
  Size(int width, int height);
  Size();

  explicit operator bool() const { return !empty(); }
  const Size* operator&() const = delete;  // NOLINT(runtime/operator)
  Size* operator&() = delete;              // NOLINT(runtime/operator)

  Size& operator=(const Size& other);
  bool operator==(const Size& other) const;
  bool operator!=(const Size& other) const;
  bool operator<(const Size& other) const;
  bool operator<=(const Size& other) const;
  bool operator>(const Size& other) const;
  bool operator>=(const Size& other) const;
  Size operator+(const Size& other) const;
  Size operator-(const Size& other) const;
  Size operator*(int multiplier) const;
  Size operator/(int divider) const;

  bool empty() const { return width() <= 0 || height() <= 0; }
  int height() const { return data_.cy; }
  void set_height(int height) { data_.cy = height; }
  int width() const { return data_.cx; }
  void set_width(int width) { data_.cx = width; }

  int GetArea() const;

 private:
  SIZE data_;
};

}  // namespace win
}  // namespace common

COMMON_EXPORT std::ostream& operator<<(std::ostream& out,
                                       const common::win::Size& size);
COMMON_EXPORT std::ostream& operator<<(std::ostream& out,
                                       const common::win::Size* size);

#endif  // COMMON_WIN_SIZE_H_
