// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include <string>

#include "base/basictypes.h"
#include "gmock/gmock.h"
#include "evita/dom/abstract_dom_test.h"
#include "evita/dom/mock_view_impl.h"
#include "evita/dom/script_host.h"
#include "evita/dom/view_delegate.h"

namespace dom {

using ::testing::Eq;
using ::testing::_;

class EditorWindowTest : public AbstractDomTest {
 protected:
  EditorWindowTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(EditorWindowTest);
};

TEST_F(EditorWindowTest, EditorWindow_list) {
  EXPECT_CALL(*mock_view_impl(), CreateEditorWindow(_)).Times(3);
  EXPECT_SCRIPT_VALID(
      "var a = new EditorWindow();"
      "var b = new EditorWindow();"
      "var c = new EditorWindow();");
  EXPECT_SCRIPT_VALID(
      "var list = EditorWindow.list.sort(function(a, b) {"
      "  return a.id - b.id;"
      "});");
  EXPECT_SCRIPT_EQ("3", "list.length");
  EXPECT_SCRIPT_TRUE("list[0] === a");
  EXPECT_SCRIPT_TRUE("list[1] === b");
  EXPECT_SCRIPT_TRUE("list[2] === c");
}

TEST_F(EditorWindowTest, Realize) {
  EXPECT_CALL(*mock_view_impl(), CreateEditorWindow(_));
  EXPECT_SCRIPT_VALID("var sample = new EditorWindow()");
  EXPECT_SCRIPT_TRUE("sample instanceof EditorWindow");
  EXPECT_SCRIPT_TRUE("sample instanceof Window");
  EXPECT_SCRIPT_EQ("1", "sample.id");

  EXPECT_CALL(*mock_view_impl(), RealizeWindow(1));
  EXPECT_SCRIPT_VALID("sample.realize()");
}

}  // namespace dom
