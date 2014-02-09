// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/abstract_dom_test.h"

namespace {

class DocumentEventTest : public dom::AbstractDomTest {
  protected: DocumentEventTest() {
  }
  public: virtual ~DocumentEventTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(DocumentEventTest);
};

TEST_F(DocumentEventTest, ctor) {
  EXPECT_SCRIPT_VALID(
      "var event = new DocumentEvent('foo', {"
      "  bubbles: true,"
      "  cancelable: false,"
      "  errorCode: 123"
      "});");
  EXPECT_SCRIPT_TRUE("event.bubbles");
  EXPECT_SCRIPT_FALSE("event.cancelable");
  EXPECT_SCRIPT_TRUE("event.current_target == null");
  EXPECT_SCRIPT_FALSE("event.defaultPrevented");
  EXPECT_SCRIPT_TRUE("event.eventPhase == Event.PhaseType.NONE");
  EXPECT_SCRIPT_TRUE("event.timeStamp == 0");
  EXPECT_SCRIPT_TRUE("event.target == null");
  EXPECT_SCRIPT_EQ("foo", "event.type");

  // DocumentEvent
  EXPECT_SCRIPT_EQ("123", "event.errorCode");
}

}  // namespace
