// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_CSS_PROPERTIES_H_
#define EVITA_VISUALS_CSS_PROPERTIES_H_

#include <stddef.h>
#include <stdint.h>

#include "evita/visuals/css/properties_forward.h"

namespace visuals {
namespace css {

enum class PropertyId : uint32_t {
  Invalid,
{% for property in properties %}
  {{property.Name}},
{% endfor %}
};

}  // namespace css
}  // namespace visuals

#endif  // EVITA_VISUALS_CSS_PROPERTIES_H_
