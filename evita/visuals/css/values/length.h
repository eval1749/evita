// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_CSS_VALUES_LENGTH_H_
#define EVITA_VISUALS_CSS_VALUES_LENGTH_H_

#include "evita/visuals/css/values/dimension.h"

namespace visuals {
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
}  // namespace visuals

#endif  // EVITA_VISUALS_CSS_VALUES_LENGTH_H_
