// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <array>
#include <ostream>

#include "evita/visuals/css/values.h"

#include "base/logging.h"

namespace visuals {
namespace css {

{% for type in types if not type.is_primitive %}
//////////////////////////////////////////////////////////////////////
//
// CSS Value {{type.Name}}
//
enum class {{type.Name}}::Kind {
{%  for member in type.members %}
  {{member.Name}},
{% endfor %}
};

{%  for member in type.members if not member.is_keyword %}
{{type.Name}}::{{type.Name}}({{member.Parameter}} {{member.name}})
    : {{member.name}}_({{member.name}}), kind_(Kind::{{member.Name}}) {}
{% endfor %}
{# Copy constructor #}
{{type.Name}}::{{type.Name}}({{type.Parameter}} other) :
{%  for member in type.members if not member.is_keyword %}
    {{member.name}}_(other.{{member.name}}_),
{%  endfor %}
    kind_(other.kind_) {}

{{type.Name}}::{{type.Name}}(Kind kind) : kind_(kind) {}
{# Default constructor #}
{{type.Name}}::{{type.Name}}() : {{type.Name}}(Kind::{{type.initial}}) {}
{{type.Name}}::~{{type.Name}}() {}

{{type.Name}}& {{type.Name}}::operator=({{type.Parameter}} other) {
{%  for member in type.members if not member.is_keyword %}
  {{member.name}}_ = other.{{member.name}}_;
{%  endfor %}
  kind_ = other.kind_;
  return *this;
}

bool {{type.Name}}::operator==({{type.Parameter}} other) const {
  if (this == &other)
    return true;
  if (kind_ != other.kind_)
    return false;
{%  for member in type.members if not member.is_keyword %}
  if (is_{{member.name}}())
    return {{member.name}}_ == other.{{member.name}}_;
{%  endfor %}
  return true;
}

bool {{type.Name}}::operator!=({{type.Parameter}} other) const {
  return !operator==(other);
}

{#############################################################
 #
 # Getters
 #}
{%  for member in type.members if not member.is_keyword %}
{{member.Return}} {{type.Name}}::{{member.name}}() const {
  DCHECK(is_{{member.name}}());
  return {{member.name}}_;
}

{%  endfor %}
{#############################################################
 #
 # Predicates
 #}
{% for member in type.members %}
bool {{type.Name}}::is_{{member.name}}() const {
  return kind_ == Kind::{{member.Name}};
}

{% endfor %}
{#############################################################
 #
 # Static Constructors
 #}
{% for member in type.members if member.is_keyword %}
// static
{{type.Name}} {{type.Name}}::{{member.Name}}() const {
  return {{type.Name}}(Kind::{{member.Name}});
}

{% endfor %}
{#############################################################
 #
 # Printer
 #}
// To make |css::Style| printer output simpler, we don't print type name
// in value printer.
std::ostream& operator<<(std::ostream& ostream,
                         {{type.Parameter}} {{type.name}}) {
{% for member in type.members %}
  if ({{type.name}}.is_{{member.name}}())
{%  if member.is_keyword %}
    return ostream << {{member.text}};
{%  else %}
    return ostream << {{type.name}}.{{member.name}}();
{%  endif %}
{% endfor %}
  return ostream << "???";
}

{% endfor %}

}  // namespace css
}  // namespace visuals

