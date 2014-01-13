// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/abstract_dom_test.h"
#include "evita/dom/events/event_handler.h"
#include "evita/dom/mock_view_impl.h"
#include "gmock/gmock.h"

namespace {

using ::testing::_;

class EventHandlerTest : public dom::AbstractDomTest {
  protected: EventHandlerTest() {
  }
  public: virtual ~EventHandlerTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(EventHandlerTest);
};

TEST_F(EventHandlerTest, QueryClose) {
  EXPECT_CALL(*mock_view_impl(), CreateEditorWindow(_));
  EXPECT_VALID_SCRIPT(
      "var result, target;"
      "var editorWindow = new EditorWindow();"
      "EditorWindow.handleEvent = function(event) {"
      "  target = this;"
      "  result = event;"
      "}");
  view_event_handler()->QueryClose(1);
  EXPECT_SCRIPT_EQ("queryclose", "result.type");
  EXPECT_SCRIPT_TRUE("editorWindow == target");
}

}  // namespace
