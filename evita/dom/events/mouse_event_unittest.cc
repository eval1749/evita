// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/abstract_dom_test.h"

#include "evita/dom/events/mouse_event.h"
#include "evita/dom/public/api_event.h"

namespace {

class MouseEventTest : public dom::AbstractDomTest {
  protected: MouseEventTest() {
  }
  public: virtual ~MouseEventTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(MouseEventTest);
};

TEST_F(MouseEventTest, ctor) {
  v8::HandleScope handle_scope(isolate());
  EXPECT_SCRIPT_VALID("var event;"
                      "function init(x) { event = x; }");
  domapi::MouseEvent api_event;
  api_event.target_id = 1;
  api_event.type = domapi::EventType::DblClick;
  api_event.client_x = 12;
  api_event.client_y = 34;
  api_event.button = domapi::MouseButton::Left;
  api_event.alt_key = false;
  api_event.control_key = true;
  api_event.shift_key = false;
  auto const event = new dom::MouseEvent(api_event);
  EXPECT_SCRIPT_VALID_CALL("init", event->GetWrapper(isolate()));

  EXPECT_SCRIPT_TRUE("event.bubbles");
  EXPECT_SCRIPT_TRUE("event.cancelable");
  EXPECT_SCRIPT_TRUE("event.current_target == null");
  EXPECT_SCRIPT_FALSE("event.defaultPrevented");
  EXPECT_SCRIPT_TRUE("event.eventPhase == Event.PhaseType.NONE");
  EXPECT_SCRIPT_TRUE("event.timeStamp == 0");
  EXPECT_SCRIPT_TRUE("event.target == null");
  EXPECT_SCRIPT_EQ("dblclick", "event.type");

  // UiEvent
  EXPECT_SCRIPT_EQ("2", "event.detail");
  EXPECT_SCRIPT_TRUE("event.view == null");
  EXPECT_SCRIPT_TRUE("event.related_target == null");

  // MouseEvent
  EXPECT_SCRIPT_FALSE("event.altKey");
  EXPECT_SCRIPT_EQ("0", "event.button");
  EXPECT_SCRIPT_EQ("12", "event.clientX");
  EXPECT_SCRIPT_EQ("12", "event.clientX");
  EXPECT_SCRIPT_EQ("34", "event.clientY");
  EXPECT_SCRIPT_TRUE("event.ctrlKey");
  EXPECT_SCRIPT_FALSE("event.shiftKey");
}

}  // namespace
