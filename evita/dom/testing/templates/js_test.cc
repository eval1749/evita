// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/testing/abstract_dom_test.h"

namespace dom {

class {{Name}}Test : public AbstractDomTest {
 public:
  ~{{Name}}Test() override = default;

 protected:
  {{Name}}Test() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN({{Name}}Test);
};

{% for js_file in js_files %}
TEST_F({{Name}}Test, {{js_file.name}}) {
  RunFile({{'{'}}{{js_file.path_components|join(', ')}}{{'}'}});
}

{% endfor %}
}  // namespace dom
