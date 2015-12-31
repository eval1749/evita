// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_CSS_LENGTH_H_
#define EVITA_VISUALS_CSS_LENGTH_H_

namespace visuals {
namespace css {

//////////////////////////////////////////////////////////////////////
//
// Length
//
class Length final {
 public:
  explicit Length(float value);
  Length(const Length& other);
  Length();
  ~Length();

  bool operator==(const Length& other) const;
  bool operator!=(const Length& other) const;

  float value() const { return value_; }

 private:
  float value_ = 0;
};

std::ostream& operator<<(std::ostream& ostream, const Length& length);

}  // namespace css
}  // namespace visuals

#endif  // EVITA_VISUALS_CSS_LENGTH_H_
