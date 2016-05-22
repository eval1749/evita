// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_CSS_VALUES_LENGTH_H_
#define EVITA_CSS_VALUES_LENGTH_H_

#include "evita/css/values/dimension.h"

namespace css {

enum class Unit : uint32_t;

//////////////////////////////////////////////////////////////////////
//
// Length
//
class Length final : public Dimension {
 public:
  explicit Length(float value);
  Length(float number, Unit unit);
  Length();
  ~Length();

  float value() const;
};

}  // namespace css

#endif  // EVITA_CSS_VALUES_LENGTH_H_
