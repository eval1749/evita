// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_CSS_PROPERTY_EDITOR_H_
#define EVITA_VISUALS_CSS_PROPERTY_EDITOR_H_

#include "base/macros.h"
#include "evita/visuals/css/property.h"

namespace visuals {
namespace css {

//////////////////////////////////////////////////////////////////////
//
// Property::Editor
//
class Property::Editor final {
 public:
  Editor();
  ~Editor();

  void SetValue(Property* property, const Value& value);

 private:
  DISALLOW_COPY_AND_ASSIGN(Editor);
};

}  // namespace css
}  // namespace visuals

#endif  // EVITA_VISUALS_CSS_PROPERTY_EDITOR_H_
