// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_CSS_PROPERTIES_FORWARD_H_
#define EVITA_CSS_PROPERTIES_FORWARD_H_

#include <ostream>

namespace css {

// CSS Property tuples having:
//   CamelCase name
//   underscore name
//   type
//   name string
#define FOR_EACH_VISUAL_CSS_COLOR_PROPERTY(V) \
{% for property in properties if property.has_color %}
  V({{property.Name}}, {{property.name}}, {{property.type.Name}}, {{property.text}})
{%- if not loop.last %}  \
{% endif %}
{% endfor %}


#define FOR_EACH_VISUAL_CSS_LENGTH_PROPERTY(V) \
{% for property in properties if property.has_length %}
  V({{property.Name}}, {{property.name}}, {{property.type.Name}}, {{property.text}})
{%- if not loop.last %}  \
{% endif %}
{% endfor %}


#define FOR_EACH_VISUAL_CSS_PROPERTY(V) \
  FOR_EACH_VISUAL_CSS_COLOR_PROPERTY(V) \
  FOR_EACH_VISUAL_CSS_LENGTH_PROPERTY(V) \
{% for property in properties if not property.has_color and not property.has_length %}
  V({{property.Name}}, {{property.name}}, {{property.type.Name}}, {{property.text}})
{%- if not loop.last %}  \
{% endif %}
{% endfor %}


// Forward CSS Property declarations
class Property;
enum class PropertyId : uint32_t;

const size_t kMaxPropertyId = {{properties|length()}};

std::ostream& operator<<(std::ostream& ostream, PropertyId property_id);

}  // namespace css

#endif  // EVITA_CSS_PROPERTIES_FORWARD_H_
