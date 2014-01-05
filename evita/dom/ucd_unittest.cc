// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include <string>

#include "base/basictypes.h"
#include "evita/dom/abstract_dom_test.h"

namespace {

class UcdTest : public dom::AbstractDomTest {
  protected: UcdTest() = default;
  public: virtual ~UcdTest() = default;
  DISALLOW_COPY_AND_ASSIGN(UcdTest);
};

TEST_F(UcdTest, Basic) {
  EXPECT_SCRIPT_EQ("65536", "UCD.length");
  EXPECT_SCRIPT_TRUE("UCD['\\n'.charCodeAt(0)].bc == UCD.B");
  EXPECT_SCRIPT_TRUE("UCD['\\n '.charCodeAt(0)].gc == UCD.Cc");
  EXPECT_SCRIPT_TRUE("UCD[' '.charCodeAt(0)].bc == UCD.WS");
  EXPECT_SCRIPT_TRUE("UCD[' '.charCodeAt(0)].gc == UCD.Zs");
  EXPECT_SCRIPT_TRUE("UCD['5'.charCodeAt(0)].bc == UCD.EN");
  EXPECT_SCRIPT_TRUE("UCD['5'.charCodeAt(0)].gc == UCD.Nd");
  EXPECT_SCRIPT_TRUE("UCD['A'.charCodeAt(0)].bc == UCD.L");
  EXPECT_SCRIPT_TRUE("UCD['A'.charCodeAt(0)].gc == UCD.Lu");
  EXPECT_SCRIPT_TRUE("UCD['z'.charCodeAt(0)].bc == UCD.L");
  EXPECT_SCRIPT_TRUE("UCD['z'.charCodeAt(0)].gc == UCD.Ll");
}

}  // namespace
