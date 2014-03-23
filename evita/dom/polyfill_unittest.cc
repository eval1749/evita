// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include <string>

#include "base/basictypes.h"
#include "gmock/gmock.h"
#include "evita/dom/abstract_dom_test.h"
#include "evita/dom/mock_view_impl.h"
#include "evita/dom/script_host.h"
#include "evita/dom/view_delegate.h"

namespace {

using ::testing::_;

class PolyfillTest : public dom::AbstractDomTest {
  protected: PolyfillTest() {
  }
  public: virtual ~PolyfillTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(PolyfillTest);
};

TEST_F(PolyfillTest, Array_find) {
  EXPECT_SCRIPT_VALID(
      "var list = [{value: 1}, {value: 2}, {value: 3}];"
      "function predicate(value) {"
      "  return function(element) { return element.value == value; };"
      "}");
  EXPECT_SCRIPT_EQ("2", "list.find(predicate(2)).value");
  EXPECT_SCRIPT_TRUE("list.find(predicate(4)) == undefined");
}

}  // namespace
