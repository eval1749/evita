// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "{{class_name}}.h"

{% for include_path in include_paths %}
#include "{{include_path}}"
{% endfor %}

namespace dom {
namespace bindings {
{% if constructor.dispatch != 'none' %}
{#
 # Example:
 #  Type1 param1;
 #  Type2 param2;
 #  if (!gin::GetNextArgument(&args, 0, false, &param1) ||
 #      !gin::GetNextArgument(&args, 0, false, &param2)) {
 #    args.ThrowWrror();
 #    return nullptr;
 #  }
 #  return new Example(param1, param2);
 #}
{% macro emit_signature(signature, indent='  ') %}
{% if signature.parameters %}
{%-  for parameter in signature.parameters %}
{{indent}}{{parameter.return_type}} {{parameter.cpp_name}};
{%   endfor %}
{{indent}}if (
{%-  for parameter in signature.parameters %}
  {% if not loop.first %}{{indent + '    '}}{% endif -%}
  !GetNextArgument(&args, 0, false, &{{parameter.cpp_name}})
  {%- if not loop.last %}{{ ' ||\n' }}{% endif %}
{%   endfor -%} ) {
{{indent + '  '}}args.ThrowError();
{{indent + '  '}}return nullptr;
{{indent}}}
{% endif %}
{{indent}}return new {{interface_name}}({{ signature.parameters | map(attribute='cpp_name') | join(', ') }});
{%- endmacro %}

using gin::internal::GetNextArgument;
{% endif %}

//////////////////////////////////////////////////////////////////////
//
// {{class_name}}
//
{{class_name}}::{{class_name}}(const char* name)
{%- if interface_parent -%}
{{ '\n  : BaseClass(name) ' }}
{%- else -%}
{{ '\n  : v8_glue::WrapperInfo(name) ' }}
{%- endif -%} {
}

{{class_name}}::~{{class_name}}() {
}
{% if constructor.dispatch != 'none' %}

void {{class_name}}::Construct{{interface_name}}(const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (!v8_glue::internal::IsValidConstructCall(info))
    return;
  v8_glue::internal::FinishConstructCall(info, New{{interface_name}}(info));
}

//////////////////////////////////////////////////////////////////////
//
// {{interface_name}} constructor dispatcher. This function calls one of
// followings:
{% for signature in constructor.signatures %}
//   {{loop.index}} {{interface_name}}(
{%-     for parameter in signature.parameters -%}
         {{parameter.parameter_type}} {{parameter.cpp_name}}
         {%- if not loop.last %}{{', '}}{% endif %}
{%-     endfor %})
{% endfor %}
//
{{interface_name}}* {{class_name}}::New{{interface_name}}(const v8::FunctionCallbackInfo<v8::Value>& info) {
{% if constructor.dispatch == 'single' %}
  gin::Arguments args(info);
  if (info.Length() != {{constructor.signature.parameters|count}}) {
    args.ThrowError();
    return nullptr;
  }
{{ emit_signature(constructor.signature) }}
{% elif constructor.dispatch == 'arity' %}
  gin::Arguments args(info);
  switch (info.Length()) {
{%  for signature in constructor.signatures %}
    case {{ signature.parameters | count }}: {
{{    emit_signature(signature, indent='      ') }}
    }
{%  endfor %}
  }
  args.ThrowError();
  return nullptr;
{% else %}
#error "Unsupported dispatch type {{constructor.dispatch}}"
{% endif %}
}

{% endif %}
// v8_glue::WrapperInfo
{% if has_static_member or constructor.dispatch != 'none' %}
v8::Handle<v8::FunctionTemplate>
{{class_name}}::CreateConstructorTemplate(v8::Isolate* isolate) {
{###############################
 #
 # Constructor
 #}
{% if constructor.dispatch != 'none' %}
  auto templ = v8::FunctionTemplate::New(isolate,
      &{{class_name}}::Construct{{interface_name}});
{% else %}
  auto templ = v8_glue::WrapperInfo::CreateConstructorTemplate(isolate);
{% endif %}
{% if has_static_member %}
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
{% else %}
  return templ;
{% endif %}
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
