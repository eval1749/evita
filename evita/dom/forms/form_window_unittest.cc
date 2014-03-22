// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/abstract_dom_test.h"

#include "evita/dom/mock_view_impl.h"

namespace {

class FormWindowTest : public dom::AbstractDomTest {
  protected: FormWindowTest() {
  }
  public: virtual ~FormWindowTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(FormWindowTest);
};

TEST_F(FormWindowTest, ctor) {
  EXPECT_SCRIPT_VALID(
      "var form = new Form('form1');"
      "var sample = new FormWindow(form);");
  EXPECT_SCRIPT_TRUE("sample instanceof Window");
  EXPECT_SCRIPT_TRUE("sample.form === form");
}

}  // namespace
