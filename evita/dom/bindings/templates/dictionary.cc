// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ginx_{{name}}.h"

{% for include_path in cc_include_paths %}
#include "{{include_path}}"
{% endfor %}

namespace dom {

{{name}}::{{name}}()
{% for member in members if member.has_default_value %}
    {% if loop.first %}{{'    : '}}{% else %}{{'      '}}{% endif -%}
    {{member.cc_name}}_({{member.default_value}}){% if not loop.last %}{{',\n'}}{% endif %}
{% endfor %} {
}

{{name}}::~{{name}}() {
}

// dom::Dictionary
Dictionary::HandleResult {{name}}::HandleKeyValue(
    v8::Local<v8::Value> key, v8::Local<v8::Value> value) {
{% for member in members %}
{%  if member.is_nullable %}
  if (v8Strings::{{member.name}}.Get(isolate())->Equals(key)) {
    {{member.from_v8_type}} maybe_{{member.cc_name}};
    if (!gin::ConvertFromV8(isolate(), value, &maybe_{{member.cc_name}}))
        return HandleResult::CanNotConvert;
    {{member.cc_name}}_ = maybe_{{member.cc_name}};
    return HandleResult::Succeeded;
  }
{%  else %}
  if (v8Strings::{{member.name}}.Get(isolate())->Equals(key)) {
    return gin::ConvertFromV8(isolate(), value, &{{member.cc_name}}_) ?
        HandleResult::Succeeded : HandleResult::CanNotConvert;
  }
{%  endif %}
{% endfor %}
{% if parent_name %}
  return {{parent_name}}::HandleKeyValue(key, value);
{% else %}
  return HandleResult::NotFound;
{% endif %}
}

}  // namespace dom
