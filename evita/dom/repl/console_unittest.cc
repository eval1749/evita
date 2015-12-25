// Copyright (c) 2013 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "base/macros.h"
#include "evita/dom/testing/abstract_dom_test.h"

namespace {

class ConsoleTest : public dom::AbstractDomTest {
 public:
  ~ConsoleTest() override = default;

 protected:
  ConsoleTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(ConsoleTest);
};

TEST_F(ConsoleTest, log) {
  EXPECT_SCRIPT_VALID(
      "function consoleContents() {"
      "  var doc = console.document;"
      "  var range = new Range(doc, 0, doc.length);"
      "  return range.text;"
      "}");

  EXPECT_SCRIPT_VALID("console.log()");
  EXPECT_SCRIPT_EQ("\n", "consoleContents()");

  EXPECT_SCRIPT_VALID("console.log('foo bar')");
  EXPECT_SCRIPT_EQ("\nfoo bar\n", "consoleContents()");

  EXPECT_SCRIPT_VALID("console.clear()");
  EXPECT_SCRIPT_EQ("", "consoleContents()");
}

}  // namespace
