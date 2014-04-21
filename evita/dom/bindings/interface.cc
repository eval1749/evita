// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "{{class_name}}.h"

{% for include_path in include_paths %}
#include "{{include_path}}"
{% endfor %}

namespace dom {
namespace bindings {
{#######################################################################
 #
 # emit_arguments
 #}
{% macro emit_arguments(signature) %}
{%-  for parameter in signature.parameters -%}
  param{{ loop.index0 }}{% if not loop.last %}{{', '}}{% endif %}
{%-  endfor %}
{%- endmacro %}
{#
 # Example:
 #  Type0 param0;
 #  auto const arg0 = info[0];
 #  if (!gin::ConvertFromV8(isolate, arg0, &param0)) {
 #    ThrowArgumentError(isolate, arg0, 0);
 #    return nullptr;
 #  }
 #  Type1 param1;
 #  auto const arg1 = info[1];
 #  if (!gin::ConvertFromV8(isolate, arg1, &param1)) {
 #    ThrowArgumentError(isolate, arg1, 1);
 #    return nullptr;
 #  }
 #  return new Example(param1, param2);
 #}
{% macro emit_constructor_signature(signature, indent='  ') %}
{% if signature.parameters %}
{%-  for parameter in signature.parameters %}
{{indent}}{{parameter.from_v8_type}} param{{ loop.index0 }};
{{indent}}auto const arg{{ loop.index0 }} = info[{{ loop.index0 }}];
{{indent}}if (!gin::ConvertFromV8(isolate, arg{{ loop.index0 }}, &param{{ loop.index0 }})) {
{{indent}}  ThrowArgumentError(isolate, "{{parameter.type}}", arg{{ loop.index0 }}, {{ loop.index0 }});
{{indent}}  return nullptr;
{{indent}}}
{% endfor %}
{% endif %}
{{indent}}return new {{interface_name}}({{ emit_arguments(signature) }});
{%- endmacro %}
{#######################################################################
 #
 # emit_method
 #}
{% macro emit_method(method) %}
void {{class_name}}::{{method.cpp_name}}(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  auto const isolate = info.GetIsolate();
{% if not method.is_static %}
  {{interface_name}}* impl = nullptr;
  if (!gin::ConvertFromV8(isolate, info.This(), &impl)) {
    ThrowReceiverError(isolate, "{{interface_name}}", info.This());
    return;
  }
{% endif %}
{% if method.dispatch == 'single' %}
  if (info.Length() != {{method.min_arity}}) {
    ThrowArityError(isolate, {{method.min_arity}}, {{method.min_arity}}, info.Length());
    return;
  }
{{ emit_method_signature(method.signature) }}
{% elif method.dispatch == 'arity' %}
  switch (info.Length()) {
{%  for signature in method.signatures %}
    case {{ signature.parameters | count }}: {
{{    emit_method_signature(signature, indent='      ') }}
      return;
    }
{%  endfor %}
  }
  ThrowArityError(isolate, {{ method.min_arity }}, {{ method.max_arity }}, info.Length());
{% else %}
#error "Unsupported dispatch type {{method.dispatch}}"
{% endif %}
}
{% endmacro %}
{#######################################################################
 #
 # emit_method_signature
 #}
{%- macro emit_method_signature(signature, indent='  ') %}
{% if signature.parameters %}
{%-  for parameter in signature.parameters %}
{{indent}}{{parameter.from_v8_type}} param{{ loop.index0 }};
{{indent}}auto const arg{{ loop.index0 }} = info[{{ loop.index0 }}];
{{indent}}if (!gin::ConvertFromV8(isolate, arg{{ loop.index0 }}, &param{{ loop.index0 }})) {
{{indent}}  ThrowArgumentError(isolate, "{{parameter.type}}", arg{{ loop.index0 }}, {{ loop.index0 }});
{{indent}}  return;
{{indent}}}
{% endfor %}
{% endif %}
{% if signature.is_static %}
{%   set callee = interface_name + '::' + signature.cpp_name %}
{% else %}
{%   set callee = 'impl->' + signature.cpp_name %}
{% endif %}
{%- if signature.return_type == 'void' %}
{{indent}}{{callee}}({{ emit_arguments(signature) }});
{%- else %}
{{indent}}info.GetReturnValue().Set(gin::ConvertToV8(isolate,
{{indent}}    {{callee}}({{ emit_arguments(signature) }})));
{%- endif %}
{%- endmacro %}
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
void {{class_name}}::Construct{{interface_name}}(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
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
{{interface_name}}* {{class_name}}::New{{interface_name}}(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  auto const isolate = info.GetIsolate();
{% if constructor.dispatch == 'single' %}
  if (info.Length() != {{constructor.min_arity}}) {
    ThrowArityError(isolate, {{constructor.min_arity}}, {{constructor.min_arity}}, info.Length());
    return nullptr;
  }
{{ emit_constructor_signature(constructor.signature) }}
{% elif constructor.dispatch == 'arity' %}
  switch (info.Length()) {
{%  for signature in constructor.signatures %}
    case {{ signature.parameters | count }}: {
{{    emit_constructor_signature(signature, indent='      ') }}
    }
{%  endfor %}
  }
  ThrowArityError(isolate, {{ constructor.min_arity }}, {{ constructor.max_arity }}, info.Length());
  return nullptr;
{% else %}
#error "Unsupported dispatch type {{constructor.dispatch}}"
{% endif %}
}

{% endif %}
{#############################################################
 #
 # Static attributes
 #}
{% for attribute in attributes if attribute.is_static %}
{%  if loop.first %}
// Static attributes
{%  endif %}
void {{class_name}}::Get_{{attribute.cpp_name}}(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  auto const isolate = info.GetIsolate();
  if (info.Length() != 0) {
    ThrowArityError(isolate, 0, 0, info.Length());
    return;
  }
  info.GetReturnValue().Set(gin::ConvertToV8(isolate,
      {{interface_name}}::{{attribute.cpp_name}}()));
}

{%  if not attribute.is_read_only %}
void {{class_name}}::Set_{{attribute.cpp_name}}(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  auto const isolate = info.GetIsolate();
  if (info.Length() != 1) {
    ThrowArityError(isolate, 1, 1, info.Length());
    return;
  }
  {{attribute.from_v8_type}} new_value;
  if (!gin::ConvertFromV8(isolate, info[0], &new_value)) {
    ThrowArgumentError(isolate, "{{attribute.type}}", info.info[0], 0);
    return;
  }
  {{interface_name}}::set_{{attribute.cpp_name}}(new_value);
}

{%  endif %}
{% endfor %}
{#############################################################
 #
 # Static methods
 #}
{% for method in methods if method.is_static %}
{%  if loop.first %}
// Static methods
{%  endif %}
{{emit_method(method)}}
{% endfor %}
{#############################################################
 #
 # Instance attributes
 #}
{% for attribute in attributes if not attribute.is_static %}
{%  if loop.first %}
// Instance attributes
{%  endif %}
void {{class_name}}::Get_{{attribute.cpp_name}}(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  auto const isolate = info.GetIsolate();
  {{interface_name}}* impl = nullptr;
  if (!gin::ConvertFromV8(isolate, info.This(), &impl)) {
    ThrowReceiverError(isolate, "{{interface_name}}", info.This());
    return;
  }
  info.GetReturnValue().Set(gin::ConvertToV8(isolate,
      impl->{{attribute.cpp_name}}()));
}

{%  if not attribute.is_read_only %}
void {{class_name}}::Set_{{attribute.cpp_name}}(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  auto const isolate = info.GetIsolate();
  if (info.Length() != 1) {
    ThrowArityError(isolate, 1, 1, info.Length());
    return;
  }
  {{interface_name}}* impl = nullptr;
  if (!gin::ConvertFromV8(isolate, info.This(), &impl)) {
    ThrowReceiverError(isolate, "{{interface_name}}", info.This());
    return;
  }
  {{attribute.from_v8_type}} new_value;
  if (!gin::ConvertFromV8(isolate, info[0], &new_value)) {
    ThrowArgumentError(isolate, "{{attribute.type}}", info[0], 0);
    return;
  }
  impl->set_{{attribute.cpp_name}}(new_value);
}

{%  endif %}
{% endfor %}
{#############################################################
 #
 # Instance methods
 #}
{% for method in methods if not method.is_static %}
{%  if loop.first %}
// Instance methods
{%  endif %}
{{emit_method(method)}}
{% endfor %}
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
 # Static attributes
 #}
{% for attribute in attributes if attribute.is_static %}
{% if loop.first %}
  // static attributes
{% endif %}
{%   if attribute.is_read_only %}
  templ->SetAccessorProperty(gin::StringToSymbol(isolate, "{{attribute.name}}"),
      v8::FunctionTemplate::New(isolate, &{{class_name}}::Get_{{attribute.cpp_name}}));
{%    else %}
  templ->SetAccessorProperty(gin::StringToSymbol(isolate, "{{attribute.name}}"),
      v8::FunctionTemplate::New(isolate, &{{class_name}}::Get_{{attribute.cpp_name}}),
      v8::FunctionTemplate::New(isolate, &{{class_name}}::Set_{{attribute.cpp_name}}));
{%    endif %}
{%  endfor %}
{###############################
 #
 # Static methods
 #}
{% for method in methods if method.is_static %}
{% if loop.first %}
  // static methods
{% endif %}
  templ->Set(gin::StringToSymbol(isolate, "{{method.name}}"),
      v8::FunctionTemplate::New(isolate, &{{class_name}}::{{method.cpp_name}}));
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
  auto const isolate = builder.isolate();
  auto const templ = builder.GetTemplate();
{% for attribute in attributes if not attribute.is_static %}
{% if loop.first %}
  // attributes
{% endif %}
{%   if attribute.is_read_only %}
  templ->SetAccessorProperty(gin::StringToSymbol(isolate, "{{attribute.name}}"),
      v8::FunctionTemplate::New(isolate, &{{class_name}}::Get_{{attribute.cpp_name}}));
{%   else %}
  templ->SetAccessorProperty(gin::StringToSymbol(isolate, "{{attribute.name}}"),
      v8::FunctionTemplate::New(isolate, &{{class_name}}::Get_{{attribute.cpp_name}}),
      v8::FunctionTemplate::New(isolate, &{{class_name}}::Set_{{attribute.cpp_name}}));
{%   endif %}
{%  endfor %}
{% for method in methods if not method.is_static %}
{% if loop.first %}
  // methods
{% endif %}
  templ->Set(gin::StringToSymbol(isolate, "{{method.name}}"),
      v8::FunctionTemplate::New(isolate, &{{class_name}}::{{method.cpp_name}}));
{% endfor %}
}

{% endif %}
}  // namespace bindings

using namespace bindings;
DEFINE_SCRIPTABLE_OBJECT({{interface_name}}, {{class_name}});

}  // namespace dom
