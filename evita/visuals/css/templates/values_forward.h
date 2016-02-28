// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_CSS_VALUES_FORWARD_H_
#define EVITA_VISUALS_CSS_VALUES_FORWARD_H_

#include <iosfwd>

namespace visuals {
namespace css {

//////////////////////////////////////////////////////////////////////
//
// Keyword
//
#define FOR_EACH_VISUAL_CSS_KEYWORD_VALUE(V) \
{% for keyword in keywords %}
  V({{keyword.Name}}, {{keyword.name}}){% if not loop.last%}{{' \\\n'}}{% endif %}
{% endfor %}

// CSS Value tuples having:
//   CamelCase name
//   underscore name
//   name string
#define FOR_EACH_VISUAL_CSS_VALUES(V) \
{% for type in types %}
  V({{type.Name}}, {{type.name}}, {{type.text}})
{%- if not loop.last %}  \
{% endif %}
{% endfor %}


// Forward class declarations representing CSS value
{% for type in types %}
class {{type.Name}};
{% endfor %}

enum class Keyword;
enum class DeprecatedValueType;

std::ostream& operator<<(std::ostream& ostream, Keyword keyword);

}  // namespace css
}  // namespace visuals

#endif  // EVITA_VISUALS_CSS_VALUES_FORWARD_H_
