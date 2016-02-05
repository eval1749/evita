// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/testing/abstract_dom_test.h"

#include "evita/dom/bindings/ginx_FocusEventInit.h"
#include "evita/dom/events/focus_event.h"

namespace dom {

class FocusEventTest : public AbstractDomTest {
 public:
  ~FocusEventTest() override = default;

 protected:
  FocusEventTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(FocusEventTest);
};

TEST_F(FocusEventTest, ctor) {
  RunnerScope runner_scope(this);
  EXPECT_SCRIPT_VALID(
      "var event;"
      "function init(x) { event = x; }");
  dom::FocusEventInit init_dict;
  init_dict.set_bubbles(true);
  auto const event = new dom::FocusEvent(L"focusin", init_dict);
  EXPECT_SCRIPT_VALID_CALL("init", event->GetWrapper(isolate()));
  EXPECT_SCRIPT_TRUE("event.bubbles");
  EXPECT_SCRIPT_FALSE("event.cancelable");
  EXPECT_SCRIPT_TRUE("event.currentTarget == null");
  EXPECT_SCRIPT_FALSE("event.defaultPrevented");
  EXPECT_SCRIPT_TRUE("event.eventPhase == Event.PhaseType.NONE");
  EXPECT_SCRIPT_TRUE("event.timeStamp == 0");
  EXPECT_SCRIPT_TRUE("event.target == null");
  EXPECT_SCRIPT_EQ("focusin", "event.type");
  EXPECT_SCRIPT_EQ("0", "event.detail");
  EXPECT_SCRIPT_TRUE("event.view == null");
  EXPECT_SCRIPT_TRUE("event.related_target == null");
}

}  // namespace dom
