// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/abstract_dom_test.h"

#include "evita/dom/events/focus_event.h"

namespace {

class FocusEventTest : public dom::AbstractDomTest {
  protected: FocusEventTest() {
  }
  public: virtual ~FocusEventTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(FocusEventTest);
};

TEST_F(FocusEventTest, ctor) {
  v8::HandleScope handle_scope(isolate());
  EXPECT_SCRIPT_VALID("var event;"
                      "function init(x) { event = x; }");
  auto const event = new dom::FocusEvent(L"focusin", nullptr);
  EXPECT_SCRIPT_VALID_CALL("init", event->GetWrapper(isolate()));
  EXPECT_SCRIPT_TRUE("event.bubbles");
  EXPECT_SCRIPT_FALSE("event.cancelable");
  EXPECT_SCRIPT_TRUE("event.current_target == null");
  EXPECT_SCRIPT_FALSE("event.defaultPrevented");
  EXPECT_SCRIPT_TRUE("event.eventPhase == Event.PhaseType.NONE");
  EXPECT_SCRIPT_TRUE("event.timeStamp == 0");
  EXPECT_SCRIPT_TRUE("event.target == null");
  EXPECT_SCRIPT_EQ("focusin", "event.type");
  EXPECT_SCRIPT_EQ("0", "event.detail");
  EXPECT_SCRIPT_TRUE("event.view == null");
  EXPECT_SCRIPT_TRUE("event.related_target == null");
}

}  // namespace
