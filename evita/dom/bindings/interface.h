// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_gen_evita_bindings_{{class_name}}_h)
#define INCLUDE_gen_evita_bindings_{{class_name}}_h

{% if base_class_include %}
#include "{{base_class_include}}"
{% endif %}
#include "evita/v8_glue/wrapper_info.h"

namespace dom {

class {{interface_name}};

namespace bindings {

class {{class_name}} final :
{%- if interface_parent -%}
    public v8_glue::DerivedWrapperInfo<{{interface_name}}, {{interface_parent}}>
{%- else -%}
    public v8_glue::WrapperInfo
{%- endif %} {
  public: {{class_name}}(const char* name);
  public: virtual ~{{class_name}}();
{#############################################################
 #
 # Constructor
 #}
{% if constructor.dispatch != 'none' %}

  // Constructor
  private: static void Construct{{interface_name}}(
      const v8::FunctionCallbackInfo<v8::Value>& info);
  private: static {{interface_name}}* New{{interface_name}}(
      const v8::FunctionCallbackInfo<v8::Value>& info);
{% endif %}
{#############################################################
 #
 # Static attributes
 #}
{% for attribute in attributes if attribute.is_static %}
{%  if loop.first %}

  // Static attributes
{%  endif %}
  private: static void Get_{{attribute.cpp_name}}(
      const v8::FunctionCallbackInfo<v8::Value>& info);
{%  if not attribute.is_read_only %}
  private: static void Set_{{attribute.cpp_name}}(
      const v8::FunctionCallbackInfo<v8::Value>& info);
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
  private: static void {{method.cpp_name}}(
      const v8::FunctionCallbackInfo<v8::Value>& info);
{% endfor %}
{#############################################################
 #
 # Attributes
 #}
{% for attribute in attributes if not attribute.is_static %}
{%  if loop.first %}

  // Instance attributes
{%  endif %}
  private: static void Get_{{attribute.cpp_name}}(
      const v8::FunctionCallbackInfo<v8::Value>& info);
{%  if not attribute.is_read_only %}
  private: static void Set_{{attribute.cpp_name}}(
      const v8::FunctionCallbackInfo<v8::Value>& info);
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
  private: static void {{method.cpp_name}}(
      const v8::FunctionCallbackInfo<v8::Value>& info);
{% endfor %}
{#############################################################
 #
 # WrapperInfo
 #}

  // v8_glue::WrapperInfo
{% if has_static_member or constructor.dispatch != 'none' %}
  private: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override;
{% endif %}
{% if need_instance_template %}
  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override;
{% endif %}

  DISALLOW_COPY_AND_ASSIGN({{class_name}});
};

}  // namespace bindings
}  // namespace dom

{% for dictionary in dictionaries %}
#include "./{{dictionary.name}}.h"
{% endfor %}

#endif //!defined(INCLUDE_gen_evita_bindings_{{class_name}}_h)
