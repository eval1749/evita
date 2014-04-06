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
namespace bindings {

class {{class_name}} :
{%- if interface_parent -%}
    public v8_glue::DerivedWrapperInfo<{{interface_name}}, {{interface_parent}}>
{%- else -%}
    public v8_glue::WrapperInfo
{%- endif %} {
  public: {{class_name}}(const char* name);
  public: virtual ~{{class_name}}();

  // v8_glue::WrapperInfo
{% if need_class_template %}
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

#endif //!defined(INCLUDE_gen_evita_bindings_{{class_name}}_h)
