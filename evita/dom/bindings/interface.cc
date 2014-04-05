// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "{{class_name}}.h"

{% for include in includes %}
#include "{{include}}"
{% endfor %}
#include "evita/dom/converter.h"
#include "evita/dom/script_host.h"
#include "evita/v8_glue/function_template_builder.h"

namespace dom {
namespace bindings {

{{class_name}}::{{class_name}}(const char* name)
    : v8_glue::WrapperInfo(name) {
}

{{class_name}}::~{{class_name}}() {
}

// v8_glue::WrapperInfo
v8::Handle<v8::FunctionTemplate> {{class_name}}::CreateConstructorTemplate(
    v8::Isolate* isolate) {
{% if constructors %}
  auto templ = v8_glue::CreateConstructorTemplate(isolate, &{interface_name}::New{{interface_name}});
{% else %}
  auto templ = v8_glue::WrapperInfo::CreateConstructorTemplate(isolate);
{% endif %}
  auto builder = v8_glue::FunctionTemplateBuilder(isolate, templ);
{% for attribute in attributes if attribute.is_static %}
{%   if attribute.is_read_only %}
  builder.SetProperty("{{attribute.name}}", &{{interface_name}}::{{attribute.name}});
{%    else %}
  builder.SetProperty("{{attribute.name}}", &{{interface_name}}::{{attribute.name}},
                                            &{{interface_name}}::set_{{attribute.name}});
{%    endif %}
{%  endfor %}
  return builder.Build();
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
  builder.SetMethod("{{method.name}}", &{{interface_name}}::{{method.cpp_name}});
{% endfor %}
}

}  // namespace bindings

using namespace bindings;
DEFINE_SCRIPTABLE_OBJECT({{interface_name}}, {{class_name}});

}  // namespace dom
