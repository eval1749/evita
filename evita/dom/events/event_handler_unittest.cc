// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/abstract_dom_test.h"
#include "evita/dom/events/event_handler.h"
#include "evita/dom/mock_view_impl.h"
#pragma warning(push)
#pragma warning(disable: 4625 4626 4365)
#include "gmock/gmock.h"
#pragma warning(pop)

namespace {

using ::testing::_;

class EventHandlerTest : public dom::AbstractDomTest {
  protected: EventHandlerTest() {
  }
  public: virtual ~EventHandlerTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(EventHandlerTest);
};

TEST_F(EventHandlerTest, DidDropWidget) {
  EXPECT_CALL(*mock_view_impl(), CreateEditorWindow(_)).Times(2);
  EXPECT_VALID_SCRIPT(
      "var result, target;"
      "var source = new EditorWindow();"
      "var editorWindow = new EditorWindow();"
      "EditorWindow.handleEvent = function(event) {"
      "  target = this;"
      "  result = event;"
      "}");
  view_event_handler()->DidDropWidget(1, 2);
  EXPECT_SCRIPT_EQ("dropwindow", "result.type");
  EXPECT_SCRIPT_TRUE("target == editorWindow");
}

TEST_F(EventHandlerTest, DidDropWidget_InvalidWindowId) {
  EXPECT_CALL(*mock_view_impl(), CreateEditorWindow(_)).Times(2);
  EXPECT_VALID_SCRIPT(
      "var result, target;"
      "var source = new EditorWindow();"
      "EditorWindow.handleEvent = function(event) {"
      "  target = this;"
      "  result = event;"
      "}");
  view_event_handler()->DidDropWidget(1, dom::kInvalidWindowId);
  EXPECT_SCRIPT_EQ("dropwindow", "result.type");
  EXPECT_SCRIPT_EQ("2", "target.id");
}

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
