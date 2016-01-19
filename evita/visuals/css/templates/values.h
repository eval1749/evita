// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_CSS_VALUES_H_
#define EVITA_VISUALS_CSS_VALUES_H_

#include <iosfwd>

#include "evita/visuals/css/values_forward.h"

{% for primitive in primitives %}
#include "evita/visuals/css/{{primitive.file_name}}.h"
{% endfor %}

namespace visuals {
namespace css {

//////////////////////////////////////////////////////////////////////
//
// Value Types
//
enum class ValueType {
  // Keywords
{% for keyword in keywords %}
  {{keyword.Name}},
{% endfor %}

  // Values
{% for type in types %}
  {{type.Name}},
{% endfor %}
};

{% for type in types if not type.is_primitive %}
//////////////////////////////////////////////////////////////////////
//
// {{type.Name}} represents CSS value type {{type.Name}}
{% for type in type.members %}
{%   if type.is_keyword %}
//    {{type.name}}
{%   else %}
//    <{{type.name}}>
{%   endif %}
{% endfor %}
//
class {{type.Name}} final {
 public:
{%  for member in type.members if not member.is_keyword %}
  explicit {{type.Name}}({{member.Parameter}} {{member.name}});
{%  endfor %}
  {{type.Name}}({{type.Parameter}} other);
  {{type.Name}}();
  ~{{type.Name}}();

  {{type.Name}}& operator=({{type.Parameter}} other);

  bool operator==({{type.Parameter}} other) const;
  bool operator!=({{type.Parameter}} other) const;

{#############################################################
 #
 # Getters/Predicates/Keywords
 #}
{%  for member in type.members if not member.is_keyword %}
{%    if loop.first %}
  // Getters
{%    endif %}
  {{member.Return}} {{member.name}}() const;
{%  endfor %}

  // Predicates
{%  for member in type.members %}
  bool is_{{member.name}}() const;
{% endfor %}

  // Keywords
{%  for member in type.members if member.is_keyword %}
  static {{type.Name}} {{member.Name}}();
{%  endfor %}

{#############################################################
 #
 # Private section
 #}
 private:
  explicit {{type.Name}}(ValueType value_type);

  // Values
{%  for member in type.members if not member.is_keyword %}
  {{member.Name}} {{member.name}}_;
{%  endfor %}
  ValueType value_type_ = ValueType::{{type.initial}};
};

std::ostream& operator<<(std::ostream& ostream, {{type.Parameter}} {{type.name}});

{% endfor %}

}  // namespace css
}  // namespace visuals

#endif  // EVITA_VISUALS_CSS_VALUES_H_
