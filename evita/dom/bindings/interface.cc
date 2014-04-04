// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "{{interface_name}}.h"

#include "{{implement_h_path}}"
#include "evita/v8_glue/function_template_builder.h"

namespace dom {

DEFINE_SCRIPTABLE_OBJECT({{interface_name}}, bindings::{{class_name}});

namespace bindings {

namespace {
{{interface_name}}* New{{interface_name}}() {
  return new {{interface_name}}();
}

// v8_glue::WrapperInfo
v8::Handle<v8::FunctionTemplate> {{class_name}}::CreateConstructorTemplate(
    v8::Isolate* isolate) {
  auto templ = v8_glue::CreateConstructorTemplate(isolate,
      &New{{interface_name}});
  auto build = v8_glue::FunctionTemplateBuilder(isolate, templ);
{% for attribute in attributes if attribute.is_static %}
{%   if attribute.is_read_only %}
  builder.SetProperty("{{attribute.name}}", &{{interface_name}}::{{attribute.name}});
{%    else %}
  builder.SetProperty("{{attribute.name}}", &{{interface_name}}::{{attribute.name}},
                                            &{{interface_name}}::set_{{attribute.name}});
{%    endif %}
{%  endfor %}
  build.Build();
}

void {{class_name}}:: SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) {
{% for attribute in attributes if not attribute.is_static %}
{%   if attribute.is_read_only %}
  builder.SetProperty("{{attribute.name}}", &{{interface_name}}::{{attribute.name}});
{%   else %}
  builder.SetProperty("{{attribute.name}}",
      &{{interface_name}}::{{attribute.name}},
      &{{interface_name}}::set_{{attribute.name}});
{%   endif %}
{%  endfor %}
{% for method in methods if not method.is_static %}
  builder.SetMethod("{{method.name}}", &{{interface_name}}::{{method.name}});
{% endfor %}
}

}  // namespace bindings
}  // namespace dom
