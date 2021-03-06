// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_CSS_PROPERTY_SET_EDITOR_H_
#define EVITA_CSS_PROPERTY_SET_EDITOR_H_

#include <memory>
#include <vector>

#include "base/macros.h"
#include "evita/css/properties_forward.h"
#include "evita/css/property_set.h"

namespace css {

//////////////////////////////////////////////////////////////////////
//
// PropertySet::Editor
//
class PropertySet::Editor final {
 public:
  Editor();
  ~Editor();

  void Add(PropertySet* property_set,
           PropertyId property_id,
           const Value& value);
  // Merge |right| into |left| if |left| doesn't have property.
  void Merge(PropertySet* left, const PropertySet& right);
  void Remove(PropertySet* property_set, PropertyId property_id);
  void Set(PropertySet* property_set,
           PropertyId property_id,
           const Value& value);

#define V(Name, name, type, text)                                \
  void Set##Name(PropertySet* property_set, const Value& value); \
  void Remove##Name(PropertySet* property_set);

  FOR_EACH_VISUAL_CSS_PROPERTY(V)
#undef V

 private:
  DISALLOW_COPY_AND_ASSIGN(Editor);
};

}  // namespace css

#endif  // EVITA_CSS_PROPERTY_SET_EDITOR_H_
