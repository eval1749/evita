// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_STYLE_WIDTH_H_
#define EVITA_VISUALS_STYLE_WIDTH_H_

#include <iosfwd>

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// Width
//
class Width final {
 public:
  Width(const Width& other);
  Width();
  ~Width();

  Width& operator=(const Width& other);

  bool operator==(const Width& other) const;
  bool operator!=(const Width& other) const;

  float value() const;

  bool IsAuto() const;
  bool IsLength() const;
  bool IsPercentage() const;

  static Width Auto();
  static Width Length(float value);
  static Width Percentage(float value);

 private:
  enum class Kind;

  Width(Kind kind, float value);

  Kind kind_;
  float value_;
};

std::ostream& operator<<(std::ostream& ostream, const Width& width);

}  // namespace visuals

#endif  // EVITA_VISUALS_STYLE_WIDTH_H_
