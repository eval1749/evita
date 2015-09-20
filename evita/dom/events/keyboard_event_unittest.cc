// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/abstract_dom_test.h"

#include "evita/dom/events/keyboard_event.h"
#include "evita/dom/public/view_event.h"

namespace dom {

class KeyboardEventTest : public AbstractDomTest {
 public:
  ~KeyboardEventTest() override = default;

 protected:
  KeyboardEventTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(KeyboardEventTest);
};

TEST_F(KeyboardEventTest, ctor_event) {
  RunnerScope runner_scope(this);
  EXPECT_SCRIPT_VALID(
      "var event;"
      "function init(x) { event = x; }");
  domapi::KeyboardEvent raw_event;
  raw_event.target_id = 1;
  raw_event.event_type = domapi::EventType::KeyDown;
  raw_event.alt_key = false;
  raw_event.control_key = true;
  raw_event.meta_key = false;
  raw_event.shift_key = false;
  raw_event.location = 0;
  raw_event.key_code = 0x42;
  raw_event.repeat = false;

  auto const event = new KeyboardEvent(raw_event);
  EXPECT_SCRIPT_VALID_CALL("init", event->GetWrapper(isolate()));

  EXPECT_SCRIPT_TRUE("event instanceof KeyboardEvent");
  EXPECT_SCRIPT_TRUE("event.bubbles");
  EXPECT_SCRIPT_TRUE("event.cancelable");
  EXPECT_SCRIPT_TRUE("event.currentTarget == null");
  EXPECT_SCRIPT_FALSE("event.defaultPrevented");
  EXPECT_SCRIPT_TRUE("event.eventPhase == Event.PhaseType.NONE");
  EXPECT_SCRIPT_TRUE("event.timeStamp == 0");
  EXPECT_SCRIPT_TRUE("event.target == null");
  EXPECT_SCRIPT_EQ("keydown", "event.type");

  // UiEvent
  EXPECT_SCRIPT_EQ("0", "event.detail");
  EXPECT_SCRIPT_TRUE("event.view == null");

  // KeyboardEvent
  EXPECT_SCRIPT_FALSE("event.altKey");
  EXPECT_SCRIPT_TRUE("event.ctrlKey");
  EXPECT_SCRIPT_FALSE("event.metaKey");
  EXPECT_SCRIPT_FALSE("event.shiftKey");

  EXPECT_SCRIPT_EQ("66", "event.keyCode");
  EXPECT_SCRIPT_EQ("0", "event.location");
  EXPECT_SCRIPT_FALSE("event.repeat");
}

TEST_F(KeyboardEventTest, ctor_init_dict) {
  EXPECT_SCRIPT_VALID(
      "var event = new KeyboardEvent('keydown', {"
      "  bubbles: false,"
      "  cancelable: false,"
      "  detail: 3,"
      "  altKey: true, ctrlKey: false, metaKey: true, shiftKey: true,"
      "  keyCode: 66,"
      "  location: 3,"
      "  repeat: true"
      "});");
  EXPECT_SCRIPT_TRUE("event instanceof KeyboardEvent");
  EXPECT_SCRIPT_FALSE("event.bubbles");
  EXPECT_SCRIPT_FALSE("event.cancelable");
  EXPECT_SCRIPT_TRUE("event.currentTarget == null");
  EXPECT_SCRIPT_FALSE("event.defaultPrevented");
  EXPECT_SCRIPT_TRUE("event.eventPhase == Event.PhaseType.NONE");
  EXPECT_SCRIPT_TRUE("event.timeStamp == 0");
  EXPECT_SCRIPT_TRUE("event.target == null");
  EXPECT_SCRIPT_EQ("keydown", "event.type");

  // UiEvent
  EXPECT_SCRIPT_EQ("3", "event.detail");
  EXPECT_SCRIPT_TRUE("event.view == null");

  // KeyboardEvent
  EXPECT_SCRIPT_TRUE("event.altKey");
  EXPECT_SCRIPT_FALSE("event.ctrlKey");
  EXPECT_SCRIPT_TRUE("event.metaKey");
  EXPECT_SCRIPT_TRUE("event.shiftKey");

  EXPECT_SCRIPT_EQ("66", "event.keyCode");

  EXPECT_SCRIPT_EQ("3", "event.location");
  EXPECT_SCRIPT_TRUE("event.repeat");
}

}  // namespace dom
