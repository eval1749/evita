// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/visuals/css_value_parsers.h"

#include "common/maybe.h"
#include "evita/visuals/css/values.h"

namespace dom {

template <typename T> using Maybe = common::Maybe<T>;

{% for type in types if not type.is_primitive %}
using {{type.Name}} = visuals::css::{{type.Name}};
{% endfor %}

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
    return common::Just<{{type.Name}}>({{type.Name}}::{{member.Name}}());
{%  endfor %}
{%  for member in type.members if not member.is_keyword %}
  {
    Maybe<{{member.Name}}> maybe_{{member.name}}(Parse{{member.Name}}(text));
    if (maybe_{{member.name}}.IsJust())
      return common::Just<{{type.Name}}>({{type.Name}}(maybe_{{member.name}}.FromJust()));
  }
{%  endfor %}
  return common::Nothing<{{type.Name}}>();
}

base::string16 Unparse{{type.Name}}(const {{type.Name}}& value) {
{%  for member in type.members if member.is_keyword %}
  if (value.is_{{member.name}}())
    return L{{member.text}};
{%  endfor %}
{%  for member in type.members if not member.is_keyword %}
  if (value.is_{{member.name}}())
    return Unparse{{member.Name}}(value.{{member.name}}());
{%  endfor %}
  return L"???";
}

{% endfor %}

}  // namespace dom
