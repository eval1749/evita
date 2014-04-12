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
{# TODO(yosi) We should include "array_buffer.h" if needed #}
#include "gin/array_buffer.h"

namespace dom {
namespace bindings {

{{class_name}}::{{class_name}}(const char* name) : {{' '}}
{%- if interface_parent -%}
    BaseClass(name)
{%- else -%}
    v8_glue::WrapperInfo(name)
{%- endif -%} {{' '}}{
}

{{class_name}}::~{{class_name}}() {
}

// v8_glue::WrapperInfo
{% if need_class_template %}
{##############################################################
 #
 # Class template
 #}
v8::Handle<v8::FunctionTemplate> {{class_name}}::CreateConstructorTemplate(
    v8::Isolate* isolate) {
{% if constructors %}
  auto templ = v8_glue::CreateConstructorTemplate(isolate, &{{interface_name}}::New{{interface_name}});
{% else %}
  auto templ = v8_glue::WrapperInfo::CreateConstructorTemplate(isolate);
{% endif %}
  auto builder = v8_glue::FunctionTemplateBuilder(isolate, templ);
{###############################
 #
 # Static properties
 #}
{% for attribute in attributes if attribute.is_static %}
{%   if attribute.is_read_only %}
  builder.SetProperty("{{attribute.name}}", &{{interface_name}}::{{attribute.cpp_name}});
{%    else %}
  builder.SetProperty("{{attribute.name}}", &{{interface_name}}::{{attribute.cpp_name}},
                                            &{{interface_name}}::set_{{attribute.cpp_name}});
{%    endif %}
{%  endfor %}
{###############################
 #
 # Static methods
 #}
{% for method in methods if method.is_static %}
  builder.SetMethod("{{method.name}}", &{{interface_name}}::{{method.cpp_name}});
{% endfor %}
  return builder.Build();
}

{% endif %}
{% if need_instance_template %}
{##############################################################
 #
 # Interface template
 #}
void {{class_name}}:: SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) {
{% for attribute in attributes if not attribute.is_static %}
{%   if attribute.is_read_only %}
  builder.SetProperty("{{attribute.name}}", &{{interface_name}}::{{attribute.cpp_name}});
{%   else %}
  builder.SetProperty("{{attribute.name}}",
      &{{interface_name}}::{{attribute.cpp_name}},
      &{{interface_name}}::set_{{attribute.cpp_name}});
{%   endif %}
{%  endfor %}
{% for method in methods if not method.is_static %}
  builder.SetMethod("{{method.name}}", &{{interface_name}}::{{method.cpp_name}});
{% endfor %}
}

{% endif %}
}  // namespace bindings

using namespace bindings;
DEFINE_SCRIPTABLE_OBJECT({{interface_name}}, {{class_name}});

}  // namespace dom
