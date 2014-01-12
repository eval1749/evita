// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "gmock/gmock.h"
#include "evita/dom/abstract_dom_test.h"
#include "evita/dom/mock_view_impl.h"

namespace {

using ::testing::_;

class WindowEventTest : public dom::AbstractDomTest {
  protected: WindowEventTest() {
  }
  public: virtual ~WindowEventTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(WindowEventTest);
};

TEST_F(WindowEventTest, ctor) {
  EXPECT_VALID_SCRIPT("var event = new WindowEvent();");
  EXPECT_SCRIPT_FALSE("event.bubbles");
  EXPECT_SCRIPT_FALSE("event.cancelable");
  EXPECT_SCRIPT_TRUE("event.current_target == null");
  EXPECT_SCRIPT_FALSE("event.defaultPrevented");
  EXPECT_SCRIPT_TRUE("event.eventPhase == Event.PhaseType.NONE");
  EXPECT_SCRIPT_TRUE("event.timeStamp == 0");
  EXPECT_SCRIPT_TRUE("event.target == null");
  EXPECT_SCRIPT_EQ("", "event.type");

  // WindowEvent
  EXPECT_SCRIPT_TRUE("event.sourceWindow == null");
}

TEST_F(WindowEventTest, initWindowEvent) {
  EXPECT_CALL(*mock_view_impl(), CreateEditorWindow(_));
  EXPECT_VALID_SCRIPT(
      "var event = new WindowEvent();"
      "var source = new EditorWindow();"
      "event.initWindowEvent('foo', true, true, source)");
  EXPECT_SCRIPT_TRUE("event.bubbles");
  EXPECT_SCRIPT_TRUE("event.cancelable");
  EXPECT_SCRIPT_TRUE("event.current_target == null");
  EXPECT_SCRIPT_FALSE("event.defaultPrevented");
  EXPECT_SCRIPT_TRUE("event.eventPhase == Event.PhaseType.NONE");
  EXPECT_SCRIPT_TRUE("event.timeStamp == 0");
  EXPECT_SCRIPT_TRUE("event.target == null");
  EXPECT_SCRIPT_EQ("foo", "event.type");

  // WindowEvent
  EXPECT_SCRIPT_TRUE("event.sourceWindow == source");
}

}  // namespace
