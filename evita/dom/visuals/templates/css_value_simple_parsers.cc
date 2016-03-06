// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/visuals/css_value_parsers.h"

#include "evita/base/maybe.h"
#include "evita/visuals/css/values.h"

namespace dom {

template <typename T> using Maybe = base::Maybe<T>;

{% for type in types if not type.is_primitive %}
using {{type.Name}} = visuals::css::{{type.Name}};
{% endfor %}

using ColorValue = visuals::css::ColorValue;
using Length = visuals::css::Length;
using Percentage = visuals::css::Percentage;
using String = visuals::css::String;

//////////////////////////////////////////////////////////////////////
//
// CSS Value Parsers
//

// TODO(eval1749): We should have CSS keyword table to map text to CSS keyword
// id instead of checking equality each by each.

{% for type in types if not type.is_primitive %}
Maybe<{{type.Name}}> Parse{{type.Name}}(base::StringPiece16 text) {
{%  for member in type.members if member.is_keyword %}
  if (text == L{{member.text}})
    return base::Just<{{type.Name}}>({{type.Name}}::{{member.Name}}());
{%  endfor %}
{%  for member in type.members if not member.is_keyword %}
  {
    Maybe<{{member.Name}}> maybe_{{member.name}}(Parse{{member.Name}}(text));
    if (maybe_{{member.name}}.IsJust())
      return base::Just<{{type.Name}}>({{type.Name}}(maybe_{{member.name}}.FromJust()));
  }
{%  endfor %}
  return base::Nothing<{{type.Name}}>();
}

base::string16 Unparse{{type.Name}}(const {{type.Name}}& property_value) {
  return property_value.value().ToString16();
}

{% endfor %}

}  // namespace dom
