// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DOM_SHAPE_DATA_H_
#define EVITA_VISUALS_DOM_SHAPE_DATA_H_

#include <stdint.h>

#include <iosfwd>
#include <vector>

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// ShapeData
//
class ShapeData final {
 public:
  explicit ShapeData(const std::vector<uint8_t>& value);
  ShapeData(const ShapeData& other);
  ShapeData(ShapeData&& other);
  ~ShapeData();

  ShapeData& operator=(const ShapeData& other);
  ShapeData& operator=(ShapeData&& other);

  bool operator==(const ShapeData& other) const;
  bool operator!=(const ShapeData& other) const;

  const std::vector<uint8_t>& value() const { return value_; }

 private:
  std::vector<uint8_t> value_;
};

std::ostream& operator<<(std::ostream& ostream, const ShapeData& data);

}  // namespace visuals

#endif  // EVITA_VISUALS_DOM_SHAPE_DATA_H_
