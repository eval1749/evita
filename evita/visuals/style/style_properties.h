// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_STYLE_STYLE_PROPERTIES_H_
#define EVITA_VISUALS_STYLE_STYLE_PROPERTIES_H_

#include <iosfwd>

#include "evita/visuals/style/style_properties_forward.h"

namespace visuals {

enum class StylePropertyId {
#define V(Name, name, type, text) Name,
  FOR_EACH_VISUAL_STYLE_PROPERTY(V)
#undef V
};

std::ostream& operator<<(std::ostream& ostream, StylePropertyId property_id);

}  // namespace visuals

#endif  // EVITA_VISUALS_STYLE_STYLE_PROPERTIES_H_
