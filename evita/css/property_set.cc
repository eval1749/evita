// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>
#include <utility>

#include "evita/css/property_set.h"

#include "base/logging.h"
#include "evita/css/properties_forward.h"
#include "evita/css/values/color_value.h"
#include "evita/css/values/dimension.h"
#include "evita/css/values/percentage.h"
#include "evita/css/values/string.h"
#include "evita/css/values/unit.h"
#include "evita/css/values/value.h"
#include "evita/css/values/value_type.h"

namespace css {

//////////////////////////////////////////////////////////////////////
//
// PropertySet
//
PropertySet::PropertySet(const PropertySet& other)
    : properties_(other.properties_) {}

PropertySet::PropertySet(PropertySet&& other)
    : properties_(std::move(other.properties_)) {}

PropertySet::PropertySet() {}
PropertySet::~PropertySet() {}

PropertySet& PropertySet::operator=(const PropertySet& other) {
  properties_ = other.properties_;
  return *this;
}

PropertySet& PropertySet::operator=(PropertySet&& other) {
  properties_ = std::move(other.properties_);
  return *this;
}

bool PropertySet::operator==(const PropertySet& other) const {
  if (this == &other)
    return true;
  if (properties_.size() != other.properties_.size())
    return false;
  for (const auto& property : *this) {
    const auto& it = std::find(other.begin(), other.end(), property);
    if (it == other.end())
      return false;
  }
  return true;
}

bool PropertySet::operator!=(const PropertySet& other) const {
  return !operator==(other);
}

bool PropertySet::Contains(PropertyId property_id) const {
  for (const auto& property : *this) {
    if (property.id() == property_id)
      return true;
  }
  return false;
}

Value PropertySet::ValueOf(PropertyId property_id) const {
  for (const auto& property : *this) {
    if (property.id() == property_id)
      return std::move(property.value());
  }
  return Value();
}

std::ostream& operator<<(std::ostream& ostream,
                         const PropertySet& property_set) {
  ostream << '{';
  auto* delimiter = "";
  for (const auto& property : property_set) {
    ostream << delimiter << property.id() << ':' << property.value();
    delimiter = ", ";
  }
  return ostream << '}';
}

}  // namespace css
