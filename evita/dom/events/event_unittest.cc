// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/testing/abstract_dom_test.h"

namespace dom {

class EventTest : public AbstractDomTest {
 public:
  ~EventTest() override = default;

 protected:
  EventTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(EventTest);
};

TEST_F(EventTest, ctor) {
  EXPECT_SCRIPT_VALID("var event = new Event('foo');");
  EXPECT_SCRIPT_FALSE("event.bubbles");
  EXPECT_SCRIPT_FALSE("event.cancelable");
  EXPECT_SCRIPT_TRUE("event.currentTarget == null");
  EXPECT_SCRIPT_FALSE("event.defaultPrevented");
  EXPECT_SCRIPT_TRUE("event.eventPhase == Event.PhaseType.NONE");
  EXPECT_SCRIPT_TRUE("event.timeStamp == 0");
  EXPECT_SCRIPT_TRUE("event.target == null");
  EXPECT_SCRIPT_EQ("foo", "event.type");
}

TEST_F(EventTest, initEvent) {
  EXPECT_SCRIPT_VALID(
      "var event = new Event('foo', {"
      "     bubbles: true,"
      "     cancelable: true"
      "});");
  EXPECT_SCRIPT_TRUE("event.bubbles");
  EXPECT_SCRIPT_TRUE("event.cancelable");
  EXPECT_SCRIPT_TRUE("event.currentTarget == null");
  EXPECT_SCRIPT_FALSE("event.defaultPrevented");
  EXPECT_SCRIPT_TRUE("event.eventPhase == Event.PhaseType.NONE");
  EXPECT_SCRIPT_TRUE("event.timeStamp == 0");
  EXPECT_SCRIPT_TRUE("event.target == null");
  EXPECT_SCRIPT_EQ("foo", "event.type");
}

TEST_F(EventTest, preventDefault) {
  EXPECT_SCRIPT_VALID(
      "function testIt(cancelable) {"
      "   var event = new Event('foo', {cancelable: cancelable});"
      "   event.preventDefault();"
      "   return event.defaultPrevented;"
      "}");
  EXPECT_SCRIPT_TRUE("testIt(true)");
  EXPECT_SCRIPT_FALSE("testIt(false)")
      << "We can't prevent default if event isn't cancelable.";
}

}  // namespace dom
