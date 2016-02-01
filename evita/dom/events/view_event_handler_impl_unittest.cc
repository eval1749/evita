// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/view_event_handler_impl.h"
#include "evita/dom/testing/abstract_dom_test.h"
#include "evita/dom/testing/mock_view_impl.h"
#include "gmock/gmock.h"

namespace dom {

using ::testing::_;

class ViewEventHandlerImplTest : public AbstractDomTest {
 public:
  ~ViewEventHandlerImplTest() override = default;

 protected:
  ViewEventHandlerImplTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(ViewEventHandlerImplTest);
};

TEST_F(ViewEventHandlerImplTest, DidDropWidget) {
  EXPECT_CALL(*mock_view_impl(), CreateEditorWindow(_)).Times(2);
  EXPECT_SCRIPT_VALID(
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

TEST_F(ViewEventHandlerImplTest, DidDropWidget_InvalidWindowId) {
  EXPECT_CALL(*mock_view_impl(), CreateEditorWindow(_)).Times(2);
  EXPECT_SCRIPT_VALID(
      "var result, target;"
      "var source = new EditorWindow();"
      "EditorWindow.handleEvent = function(event) {"
      "  target = this;"
      "  result = event;"
      "}");
  view_event_handler()->DidDropWidget(1, domapi::kInvalidWindowId);
  EXPECT_SCRIPT_EQ("dropwindow", "result.type");
  EXPECT_SCRIPT_EQ("2", "target.id");
}

TEST_F(ViewEventHandlerImplTest, QueryClose) {
  EXPECT_CALL(*mock_view_impl(), CreateEditorWindow(_));
  EXPECT_SCRIPT_VALID(
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

}  // namespace dom
