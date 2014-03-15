// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/abstract_dom_test.h"
#include "evita/dom/windows/point.h"

namespace {

class PointTest : public dom::AbstractDomTest {
  protected: PointTest() {
  }
  public: virtual ~PointTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(PointTest);
};

TEST_F(PointTest, Constructor) {
  EXPECT_SCRIPT_VALID("var point = new Point(1234.0, 4567.0);");
  EXPECT_SCRIPT_EQ("1234", "point.x");
  EXPECT_SCRIPT_EQ("4567", "point.y");
}

TEST_F(PointTest, Wrapper) {
  RunnerScope runner_scope(this);
  EXPECT_SCRIPT_VALID("var point;"
                      "function init(x) { point = x; }");
  auto const point = new dom::Point(1234.0f, 4567.0f);
  EXPECT_SCRIPT_VALID_CALL("init", point->GetWrapper(isolate()));
  EXPECT_SCRIPT_EQ("1234", "point.x");
  EXPECT_SCRIPT_EQ("4567", "point.y");
}

}  // namespace
