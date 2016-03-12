// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <cmath>
#include <unordered_set>

#include "evita/visuals/css/property_set_editor.h"

#include "base/logging.h"
#include "evita/visuals/css/properties.h"
#include "evita/visuals/css/property_editor.h"
#include "evita/visuals/css/property_set.h"
#include "evita/visuals/css/values/color_value.h"
#include "evita/visuals/css/values/dimension.h"
#include "evita/visuals/css/values/percentage.h"
#include "evita/visuals/css/values/string.h"
#include "evita/visuals/css/values/value.h"
#include "evita/visuals/css/values/value_type.h"

namespace visuals {
namespace css {

const auto kUnitBits = 5;

using Editor = PropertySet::Editor;

//////////////////////////////////////////////////////////////////////
//
// Editor
//
Editor::Editor() {}
Editor::~Editor() {}

void Editor::Add(PropertySet* property_set,
                 PropertyId property_id,
                 const Value& value) {
  DCHECK(!property_set->Contains(property_id))
      << "Property '" << property_id << "' is already in " << *property_set;
  property_set->properties_.emplace_back(
      std::move(Property(property_id, value)));
}

void Editor::Merge(PropertySet* left, const PropertySet& right) {
  std::unordered_set<PropertyId> left_set;
  for (const auto& left_property : *left)
    left_set.insert(left_property.id());
  for (const auto& right_property : right) {
    if (left_set.count(right_property.id()))
      continue;
    Add(left, right_property.id(), right_property.value());
  }
}

void Editor::Remove(PropertySet* property_set, PropertyId property_id) {
  auto& properties = property_set->properties_;
  const auto& it = std::find_if(properties.begin(), properties.end(),
                                [property_id](const Property& property) {
                                  return property.id() == property_id;
                                });
  DCHECK(it != properties.end()) << "Property '" << property_id << "' isn't in "
                                 << *property_set;
  std::move(std::next(it), properties.end(), it);
  properties.resize(properties.size() - 1);
}

void Editor::Set(PropertySet* property_set,
                 PropertyId property_id,
                 const Value& new_value) {
  auto& properties = property_set->properties_;
  const auto& it = std::find_if(properties.begin(), properties.end(),
                                [property_id](const Property& property) {
                                  return property.id() == property_id;
                                });
  if (it == properties.end()) {
    properties.emplace_back(std::move(Property(property_id, new_value)));
    return;
  }
  Property::Editor().SetValue(&*it, new_value);
}

#define V(Name, name, type, text)                                         \
  void Editor::Set##Name(PropertySet* property_set, const Value& value) { \
    Set(property_set, PropertyId::Name, value);                           \
  }                                                                       \
  void Editor::Remove##Name(PropertySet* property_set) {                  \
    Remove(property_set, PropertyId::Name);                               \
  }

FOR_EACH_VISUAL_CSS_PROPERTY(V)
#undef V

}  // namespace css
}  // namespace visuals
