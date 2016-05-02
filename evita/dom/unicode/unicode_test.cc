// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/macros.h"
#include "evita/dom/testing/abstract_dom_test.h"

namespace dom {

class UnicodTest : public dom::AbstractDomTest {
 public:
  ~UnicodTest() override = default;

 protected:
  UnicodTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(UnicodTest);
};

TEST_F(UnicodTest, File) {
  RunFile({"unicode", "unicode_test.js"});
}

TEST_F(UnicodTest, Basic) {
  EXPECT_SCRIPT_VALID("var u = Unicode");
  EXPECT_SCRIPT_EQ("1114112", "u.UCD.length");
  EXPECT_SCRIPT_TRUE("u.UCD['\\n '.charCodeAt(0)].category == u.Category.Cc");
  EXPECT_SCRIPT_TRUE("u.UCD[' '.charCodeAt(0)].category == u.Category.Zs");
  EXPECT_SCRIPT_TRUE("u.UCD['5'.charCodeAt(0)].category == u.Category.Nd");
  EXPECT_SCRIPT_TRUE("u.UCD['A'.charCodeAt(0)].category == u.Category.Lu");
  EXPECT_SCRIPT_TRUE("u.UCD['z'.charCodeAt(0)].category == u.Category.Ll");
}

}  // namespace dom
