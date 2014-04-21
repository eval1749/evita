// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_gen_evita_bindings_{{name}}_h)
#define INCLUDE_gen_evita_bindings_{{name}}_h

#include "{{base_class_include}}"

{% for include_path in h_include_paths %}
#include "{{include_path}}"
{% endfor %}

namespace dom {

{%for class_name in class_references %}
class {{class_name}};
{% endfor %}
{% if class_references %}

{% endif %}
class {{name}} : public {{parent_name or 'Dictionary'}} {
{% for member in members %}
  private: {{member.declare_type}} {{member.cpp_name}}_;
{% endfor %}

  public: {{name}}();
  public: virtual ~{{name}}();

{% for member in members %}
  // {{member.name}}
  public: {{member.return_type}} {{member.cpp_name}}() const { return {{member.cpp_name}}_; }
  public: void set_{{member.cpp_name}}({{member.parameter_type}} new_{{member.cpp_name}}) { {{member.cpp_name}}_ = new_{{member.cpp_name}}; }

{% endfor %}
  // dom::Dictionary
  protected: virtual HandleResult HandleKeyValue(
      v8::Handle<v8::Value> key, v8::Handle<v8::Value> value) override;
};

}  // namespace dom


#endif //!defined(INCLUDE_gen_evita_bindings_{{name}}_h)
