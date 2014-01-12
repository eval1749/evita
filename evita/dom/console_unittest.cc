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
  EXPECT_VALID_SCRIPT("function consoleContents() {"
                      "  var doc = Document.find(console.DOCUMENT_NAME);"
                      "  var range = new Range(doc, 0, doc.length);"
                      "  return range.text;"
                      "}");

  EXPECT_VALID_SCRIPT("console.log()");
  EXPECT_SCRIPT_EQ("\n", "consoleContents()");

  EXPECT_VALID_SCRIPT("console.log('foo bar')");
  EXPECT_SCRIPT_EQ("\nfoo bar\n", "consoleContents()");

  EXPECT_VALID_SCRIPT("console.clear()");
  EXPECT_SCRIPT_EQ("", "consoleContents()");
}

}  // namespace
