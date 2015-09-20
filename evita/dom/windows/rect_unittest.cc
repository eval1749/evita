// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/abstract_dom_test.h"
#include "evita/dom/windows/rect.h"

namespace dom {

class RectTest : public AbstractDomTest {
 protected:
  RectTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(RectTest);
};

TEST_F(RectTest, Constructor) {
  EXPECT_SCRIPT_VALID("var rect = new Rect(12.0, 34.0, 56.0, 78.0);");
  EXPECT_SCRIPT_EQ("12", "rect.x");
  EXPECT_SCRIPT_EQ("34", "rect.y");
  EXPECT_SCRIPT_EQ("56", "rect.width");
  EXPECT_SCRIPT_EQ("78", "rect.height");

  EXPECT_SCRIPT_EQ("12", "rect.left");
  EXPECT_SCRIPT_EQ("34", "rect.top");
  EXPECT_SCRIPT_EQ("68", "rect.right");
  EXPECT_SCRIPT_EQ("112", "rect.bottom");
}

TEST_F(RectTest, Wrapper) {
  RunnerScope runner_scope(this);
  EXPECT_SCRIPT_VALID(
      "var rect;"
      "function init(x) { rect = x; }");
  domapi::FloatRect rect(12.0f, 34.0f, 56.0f, 78.0f);
  EXPECT_SCRIPT_VALID_CALL("init", gin::ConvertToV8(isolate(), rect));
  EXPECT_SCRIPT_EQ("12", "rect.x");
  EXPECT_SCRIPT_EQ("34", "rect.y");
  EXPECT_SCRIPT_EQ("56", "rect.width");
  EXPECT_SCRIPT_EQ("78", "rect.height");
}

}  // namespace dom
