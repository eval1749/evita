// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_CSS_PROPERTIES_FORWARD_H_
#define EVITA_VISUALS_CSS_PROPERTIES_FORWARD_H_

namespace visuals {
namespace css {

// CSS Property tuples having:
//   CamelCase name
//   underscore name
//   type
//   name string
#define FOR_EACH_VISUAL_CSS_PROPERTY(V) \
{% for property in properties %}
  V({{property.Name}}, {{property.name}}, {{property.type.Name}}, {{property.text}})
{%- if not loop.last %}  \
{% endif %}
{% endfor %}


// Forward CSS Property declarations
class Property;
enum class PropertyId;

#define V(Name, name, type, text) class NameProperty;
FOR_EACH_VISUAL_CSS_PROPERTY(V)
#undef V

}  // namespace css
}  // namespace visuals

#endif  // EVITA_VISUALS_CSS_PROPERTIES_FORWARD_H_
