// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/abstract_dom_test.h"

namespace {

class FormEventTest : public dom::AbstractDomTest {
  protected: FormEventTest() {
  }
  public: virtual ~FormEventTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(FormEventTest);
};

TEST_F(FormEventTest, ctor) {
  EXPECT_SCRIPT_VALID(
      "var event = new FormEvent('foo', {"
      "  bubbles: true,"
      "  cancelable: false,"
      "  data: 'bar'"
      "});");
  EXPECT_SCRIPT_TRUE("event.bubbles");
  EXPECT_SCRIPT_FALSE("event.cancelable");
  EXPECT_SCRIPT_TRUE("event.current_target == null");
  EXPECT_SCRIPT_FALSE("event.defaultPrevented");
  EXPECT_SCRIPT_TRUE("event.eventPhase == Event.PhaseType.NONE");
  EXPECT_SCRIPT_TRUE("event.timeStamp == 0");
  EXPECT_SCRIPT_TRUE("event.target == null");
  EXPECT_SCRIPT_EQ("foo", "event.type");
  EXPECT_SCRIPT_EQ("bar", "event.data");
  EXPECT_SCRIPT_TRUE("event.view == null");
}

}  // namespace
