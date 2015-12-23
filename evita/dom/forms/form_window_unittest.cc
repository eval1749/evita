// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/testing/abstract_dom_test.h"

#include "evita/dom/mock_view_impl.h"

namespace dom {

using ::testing::Eq;
using ::testing::_;

class FormWindowTest : public AbstractDomTest {
 public:
  ~FormWindowTest() override = default;

 protected:
  FormWindowTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(FormWindowTest);
};

TEST_F(FormWindowTest, ctor) {
  EXPECT_CALL(*mock_view_impl(), CreateFormWindow(Eq(2), _, _));
  EXPECT_SCRIPT_VALID(
      "var form = new Form();"
      "var sample = new FormWindow(form);");
  EXPECT_SCRIPT_TRUE("sample instanceof Window");
  EXPECT_SCRIPT_TRUE("sample.form === form");
}

}  // namespace dom
