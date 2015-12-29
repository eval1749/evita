// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_STYLE_HEIGHT_H_
#define EVITA_VISUALS_STYLE_HEIGHT_H_

#include <iosfwd>

namespace visuals {
namespace css {

//////////////////////////////////////////////////////////////////////
//
// Height
//
class Height final {
 public:
  Height(const Height& other);
  Height();
  ~Height();

  Height& operator=(const Height& other);

  bool operator==(const Height& other) const;
  bool operator!=(const Height& other) const;

  float value() const;

  bool IsAuto() const;
  bool IsLength() const;
  bool IsPercentage() const;

  static Height Auto();
  static Height Length(float value);
  static Height Percentage(float value);

 private:
  enum class Kind;

  Height(Kind kind, float value);

  Kind kind_;
  float value_;
};

std::ostream& operator<<(std::ostream& ostream, const Height& height);

}  // namespace css
}  // namespace visuals

#endif  // EVITA_VISUALS_STYLE_HEIGHT_H_
