// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_gen_evita_bindings_{{class_name}}_h)
#define INCLUDE_gen_evita_bindings_{{class_name}}_h

#include "evita/v8_glue/wrapper_info.h"

namespace dom {
namespace bindings {

class {{class_name}}: public v8_glue::WrapperInfo {
  public: {{class_name}}(const char* name);
  public: virtual ~{{class_name}}();

  // v8_glue::WrapperInfo
  private: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override;
  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override;

  DISALLOW_COPY_AND_ASSIGN({{class_name}});
};

}  // namespace bindings
}  // namespace dom

#endif //!defined(INCLUDE_gen_evita_bindings_{{class_name}}_h)
