// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include <string>

#include "base/basictypes.h"
#include "evita/dom/testing/abstract_dom_test.h"

namespace {

class UnicodTest : public dom::AbstractDomTest {
 public:
  ~UnicodTest() override = default;

 protected:
  UnicodTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(UnicodTest);
};

TEST_F(UnicodTest, Basic) {
  EXPECT_SCRIPT_VALID("var u = Unicode");
  EXPECT_SCRIPT_EQ("65536", "u.UCD.length");
  EXPECT_SCRIPT_TRUE("u.UCD['\\n'.charCodeAt(0)].bidi == u.Bidi.B");
  EXPECT_SCRIPT_TRUE("u.UCD['\\n '.charCodeAt(0)].category == u.Category.Cc");
  EXPECT_SCRIPT_TRUE("u.UCD[' '.charCodeAt(0)].bidi == u.Bidi.WS");
  EXPECT_SCRIPT_TRUE("u.UCD[' '.charCodeAt(0)].category == u.Category.Zs");
  EXPECT_SCRIPT_TRUE("u.UCD['5'.charCodeAt(0)].bidi == u.Bidi.EN");
  EXPECT_SCRIPT_TRUE("u.UCD['5'.charCodeAt(0)].category == u.Category.Nd");
  EXPECT_SCRIPT_TRUE("u.UCD['A'.charCodeAt(0)].bidi == u.Bidi.L");
  EXPECT_SCRIPT_TRUE("u.UCD['A'.charCodeAt(0)].category == u.Category.Lu");
  EXPECT_SCRIPT_TRUE("u.UCD['z'.charCodeAt(0)].bidi == u.Bidi.L");
  EXPECT_SCRIPT_TRUE("u.UCD['z'.charCodeAt(0)].category == u.Category.Ll");
}

}  // namespace
