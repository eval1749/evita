// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/testing/abstract_dom_test.h"

namespace dom {

class OrderedSetTest : public AbstractDomTest {
 public:
  ~OrderedSetTest() override = default;

 protected:
  OrderedSetTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(OrderedSetTest);
};

TEST_F(OrderedSetTest, all) {
  EXPECT_SCRIPT_VALID(
      "var set = new base.OrderedSet(function(a, b) { return a < b; });"
      "for (var data = 0; data < 30; data += 5) {"
      "  set.add(data);"
      "}");

  EXPECT_SCRIPT_EQ("6", "set.size");
  EXPECT_SCRIPT_EQ("null", "set.find(-1)");
  EXPECT_SCRIPT_EQ("0", "set.find(0).data");
  EXPECT_SCRIPT_EQ("null", "set.find(1)");
  EXPECT_SCRIPT_EQ("5", "set.find(5).data");
  EXPECT_SCRIPT_EQ("25", "set.find(25).data");
  EXPECT_SCRIPT_EQ("null", "set.find(26)");
  EXPECT_SCRIPT_EQ("null", "set.find(30)");

  EXPECT_SCRIPT_EQ("0", "set.lowerBound(-1).data");
  EXPECT_SCRIPT_EQ("0", "set.lowerBound(0).data");
  EXPECT_SCRIPT_EQ("5", "set.lowerBound(1).data");
  EXPECT_SCRIPT_EQ("5", "set.lowerBound(5).data");
  EXPECT_SCRIPT_EQ("25", "set.lowerBound(25).data");
  EXPECT_SCRIPT_EQ("null", "set.lowerBound(26)");
  EXPECT_SCRIPT_EQ("null", "set.lowerBound(30)");

  EXPECT_SCRIPT_EQ("0", "set.minimum");
  EXPECT_SCRIPT_EQ("25", "set.maximum");

  EXPECT_SCRIPT_VALID(
      "var result = [];"
      "set.forEach(data => result.push(data));");
  EXPECT_SCRIPT_EQ("0 5 10 15 20 25", "result.join(' ')");

  EXPECT_SCRIPT_EQ("false", "set.remove(-1);");
  EXPECT_SCRIPT_EQ("false", "set.remove(1);");
  EXPECT_SCRIPT_EQ("false", "set.remove(11);");
  EXPECT_SCRIPT_EQ("false", "set.remove(31);");
  EXPECT_SCRIPT_EQ("true", "set.remove(10);");
  EXPECT_SCRIPT_EQ("5", "set.size");
  EXPECT_SCRIPT_EQ("null", "set.find(10)");
  EXPECT_SCRIPT_EQ("15", "set.lowerBound(10).data");

  EXPECT_SCRIPT_VALID("set.remove(0);");
  EXPECT_SCRIPT_EQ("5", "set.minimum");

  EXPECT_SCRIPT_VALID("set.remove(25);");
  EXPECT_SCRIPT_EQ("20", "set.maximum");

  EXPECT_SCRIPT_VALID("set.clear()");
  EXPECT_SCRIPT_EQ("0", "set.size");
}

}  // namespace dom
