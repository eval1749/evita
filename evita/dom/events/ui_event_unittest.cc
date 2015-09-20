// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/abstract_dom_test.h"

namespace dom {

class UiEventTest : public AbstractDomTest {
 public:
  ~UiEventTest() override = default;

 protected:
  UiEventTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(UiEventTest);
};

TEST_F(UiEventTest, ctor) {
  EXPECT_SCRIPT_VALID("var event = new UiEvent('foo');");
  EXPECT_SCRIPT_FALSE("event.bubbles");
  EXPECT_SCRIPT_FALSE("event.cancelable");
  EXPECT_SCRIPT_TRUE("event.currentTarget == null");
  EXPECT_SCRIPT_FALSE("event.defaultPrevented");
  EXPECT_SCRIPT_TRUE("event.eventPhase == Event.PhaseType.NONE");
  EXPECT_SCRIPT_TRUE("event.timeStamp == 0");
  EXPECT_SCRIPT_TRUE("event.target == null");
  EXPECT_SCRIPT_EQ("foo", "event.type");

  // UiEvent attributes
  EXPECT_SCRIPT_EQ("0", "event.detail");
  EXPECT_SCRIPT_TRUE("event.view == null");
}

TEST_F(UiEventTest, ctor_init_dict) {
  EXPECT_SCRIPT_VALID(
      "var event = new UiEvent('foo', {"
      "   bubbles: true,"
      "   cancelable: true,"
      "   detail: 42"
      "});");
  EXPECT_SCRIPT_TRUE("event.bubbles");
  EXPECT_SCRIPT_TRUE("event.cancelable");
  EXPECT_SCRIPT_TRUE("event.currentTarget == null");
  EXPECT_SCRIPT_FALSE("event.defaultPrevented");
  EXPECT_SCRIPT_TRUE("event.eventPhase == Event.PhaseType.NONE");
  EXPECT_SCRIPT_TRUE("event.timeStamp == 0");
  EXPECT_SCRIPT_TRUE("event.target == null");
  EXPECT_SCRIPT_EQ("foo", "event.type");

  // UiEvent attributes
  EXPECT_SCRIPT_EQ("42", "event.detail");
  EXPECT_SCRIPT_TRUE("event.view == null");
}

}  // namespace dom
