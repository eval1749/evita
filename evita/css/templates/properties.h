// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_CSS_PROPERTIES_H_
#define EVITA_CSS_PROPERTIES_H_

#include <stddef.h>
#include <stdint.h>

#include "evita/css/properties_forward.h"

namespace css {

enum class PropertyId : uint32_t {
  Invalid,
{% for property in properties %}
  {{property.Name}},
{% endfor %}
};

}  // namespace css

#endif  // EVITA_CSS_PROPERTIES_H_
