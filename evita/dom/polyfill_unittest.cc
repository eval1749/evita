// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include <string>

#include "base/macros.h"
#include "evita/dom/public/view_delegate.h"
#include "evita/dom/script_host.h"
#include "evita/dom/testing/abstract_dom_test.h"
#include "evita/dom/testing/mock_view_impl.h"
#include "gmock/gmock.h"

namespace {

using ::testing::_;

class PolyfillTest : public dom::AbstractDomTest {
 public:
  ~PolyfillTest() override = default;

 protected:
  PolyfillTest() = default;

 private:
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

TEST_F(PolyfillTest, Set_keys) {
  EXPECT_SCRIPT_VALID(
      "var set = new Set();"
      "set.add('one');"
      "set.add('two');"
      "set.add('three');"
      "function toArray(it) {"
      "   var array = [];"
      "   for (var value of it) { array.push(value); }"
      "   return array;"
      "}"
      "var keys = toArray(set.keys()).sort();");
  EXPECT_SCRIPT_EQ("one three two", "keys.join(' ')");
}

}  // namespace
