// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/testing/abstract_dom_test.h"

namespace dom {

class BaseTest : public AbstractDomTest {
 public:
  ~BaseTest() override = default;

 protected:
  BaseTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(BaseTest);
};

TEST_F(BaseTest, OrderedSet) {
  RunFile({"base", "ordered_set_test.js"});
}

TEST_F(BaseTest, StringUtil) {
  RunFile({"base", "strings", "string_util.js"});
}

}  // namespace dom
