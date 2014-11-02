// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include <string>

#include "base/basictypes.h"
#include "evita/dom/abstract_dom_test.h"

namespace {

class ConsoleTest : public dom::AbstractDomTest {
  protected: ConsoleTest() {
  }
  public: virtual ~ConsoleTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(ConsoleTest);
};

TEST_F(ConsoleTest, log) {
  EXPECT_SCRIPT_VALID("function consoleContents() {"
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
