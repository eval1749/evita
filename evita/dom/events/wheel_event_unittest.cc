// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/abstract_dom_test.h"

#include "evita/dom/events/wheel_event.h"
#include "evita/dom/public/view_event.h"

namespace {

class WheelEventTest : public dom::AbstractDomTest {
  protected: WheelEventTest() {
  }
  public: virtual ~WheelEventTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(WheelEventTest);
};

TEST_F(WheelEventTest, ctor) {
  RunnerScope runner_scope(this);
  EXPECT_SCRIPT_VALID("var event;"
                      "function init(x) { event = x; }");
  domapi::WheelEvent api_event;
  api_event.target_id = 1;
  api_event.event_type = domapi::EventType::Wheel;
  api_event.client_x = 12;
  api_event.client_y = 34;
  api_event.button = domapi::MouseButton::Left;
  api_event.alt_key = false;
  api_event.control_key = true;
  api_event.shift_key = false;
  api_event.delta_mode = 1;
  api_event.delta_x = 2.0;
  api_event.delta_y = 3.0;
  api_event.delta_z = 4.0;

  auto const event = new dom::WheelEvent(api_event);
  EXPECT_SCRIPT_VALID_CALL("init", event->GetWrapper(isolate()));

  EXPECT_SCRIPT_TRUE("event instanceof WheelEvent");
  EXPECT_SCRIPT_TRUE("event.bubbles");
  EXPECT_SCRIPT_TRUE("event.cancelable");
  EXPECT_SCRIPT_TRUE("event.currentTarget == null");
  EXPECT_SCRIPT_FALSE("event.defaultPrevented");
  EXPECT_SCRIPT_TRUE("event.eventPhase == Event.PhaseType.NONE");
  EXPECT_SCRIPT_TRUE("event.timeStamp == 0");
  EXPECT_SCRIPT_TRUE("event.target == null");
  EXPECT_SCRIPT_EQ("wheel", "event.type");

  // UiEvent
  EXPECT_SCRIPT_EQ("0", "event.detail");
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

  // WheelEvent
  EXPECT_SCRIPT_EQ("1", "event.deltaMode");
  EXPECT_SCRIPT_EQ("2", "event.deltaX");
  EXPECT_SCRIPT_EQ("3", "event.deltaY");
  EXPECT_SCRIPT_EQ("4", "event.deltaZ");
}

}  // namespace
