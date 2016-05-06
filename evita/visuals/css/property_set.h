// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_CSS_PROPERTY_SET_H_
#define EVITA_VISUALS_CSS_PROPERTY_SET_H_

#include <stdint.h>

#include <iosfwd>
#include <iterator>
#include <vector>

#include "base/strings/string_piece.h"
#include "evita/visuals/css/property.h"

namespace visuals {
namespace css {

class ColorValue;
class Dimension;
enum class Keyword;
class Percentage;
enum class PropertyId : uint32_t;
class String;
enum class Unit : uint32_t;
enum class ValueType : uint32_t;
class Value;

//////////////////////////////////////////////////////////////////////
//
// PropertySet
//
class PropertySet final {
 public:
  // See "evita/visuals/css/property_set_builder.h" for implementation.
  class Builder;
  // See "evita/visuals/css/property_set_editor.h" for implementation.
  class Editor;

  PropertySet(const PropertySet& other);
  PropertySet(PropertySet&& other);
  PropertySet();
  ~PropertySet();

  PropertySet& operator=(const PropertySet& other);
  PropertySet& operator=(PropertySet&& other);

  bool operator==(const PropertySet& other) const;
  bool operator!=(const PropertySet& other) const;

  std::vector<Property>::const_iterator begin() const {
    return properties_.begin();
  }
  std::vector<Property>::iterator begin() { return properties_.begin(); }
  std::vector<Property>::iterator end() { return properties_.end(); }
  std::vector<Property>::const_iterator end() const {
    return properties_.end();
  }

  bool Contains(PropertyId property_id) const;
  Value ValueOf(PropertyId property_id) const;

 private:
  friend class PropertySetTest;

  std::vector<Property> properties_;
};

std::ostream& operator<<(std::ostream& ostream,
                         const PropertySet& property_set);

}  // namespace css
}  // namespace visuals

#endif  // EVITA_VISUALS_CSS_PROPERTY_SET_H_
