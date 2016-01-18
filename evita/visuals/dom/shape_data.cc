// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>
#include <utility>

#include "evita/visuals/dom/shape_data.h"

#include "base/strings/stringprintf.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// ShapeData
//
ShapeData::ShapeData(const std::vector<uint8_t>& value) : value_(value) {}
ShapeData::ShapeData(const ShapeData& other) : ShapeData(other.value_) {}
ShapeData::ShapeData(ShapeData&& other) : value_(std::move(other.value_)) {}
ShapeData::~ShapeData() {}

ShapeData& ShapeData::operator=(const ShapeData& other) {
  value_ = other.value_;
  return *this;
}

ShapeData& ShapeData::operator=(ShapeData&& other) {
  value_ = std::move(other.value_);
  return *this;
}

bool ShapeData::operator==(const ShapeData& other) const {
  if (this == &other)
    return true;
  return value_ == other.value_;
}

bool ShapeData::operator!=(const ShapeData& other) const {
  return !operator==(other);
}

std::ostream& operator<<(std::ostream& ostream, const ShapeData& data) {
  ostream << "ShapeData(";
  auto delimiter = "";
  for (const auto& code : data.value()) {
    ostream << delimiter << base::StringPrintf("%02X", code);
    delimiter = " ";
  }
  return ostream << ')';
}

}  // namespace visuals
