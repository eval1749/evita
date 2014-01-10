// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include <string>

#include "base/basictypes.h"
#include "gmock/gmock.h"
#include "evita/dom/abstract_dom_test.h"
#include "evita/dom/mock_view_impl.h"
#include "evita/dom/script_controller.h"
#include "evita/dom/view_delegate.h"

namespace {

using ::testing::_;

class EditorTest : public dom::AbstractDomTest {
  protected: EditorTest() {
  }
  public: virtual ~EditorTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(EditorTest);
};

TEST_F(EditorTest, getFilenameForLoad) {
  EXPECT_CALL(*mock_view_impl(), CreateEditorWindow(_));
  RunScript("var filename;"
            "var window = new EditorWindow();"
            "Editor.getFilenameForLoad(window, 'dir').then(function(x) {"
            "  filename = x;"
            "});");
  EXPECT_SCRIPT_EQ("dir/foo.bar", "filename");
}

TEST_F(EditorTest, getFilenameForSave) {
  EXPECT_CALL(*mock_view_impl(), CreateEditorWindow(_));
  RunScript("var filename;"
            "var window = new EditorWindow();"
            "Editor.getFilenameForSave(window, 'dir').then(function(x) {"
            "  filename = x;"
            "});");
  EXPECT_SCRIPT_EQ("dir/foo.bar", "filename");
}

TEST_F(EditorTest, messageBox) {
  EXPECT_CALL(*mock_view_impl(), CreateEditorWindow(_));
  RunScript("var response_code;"
            "var window = new EditorWindow();"
            "Editor.messageBox(window, 'message', 'title', 3)"
            "    .then(function(x) {"
            "       response_code = x;"
            "    });");
  EXPECT_SCRIPT_EQ("3", "response_code");
}

TEST_F(EditorTest, version) {
  EXPECT_SCRIPT_EQ("5.0", "Editor.version");
}

}  // namespace

#include "base/bind.h"
#pragma warning(push)
#include "base/test/launcher/unit_test_launcher.h"
#pragma warning(disable: 4625)
#include "base/test/test_suite.h"
#pragma warning(pop)

int main(int argc, char** argv) {
  base::TestSuite test_suite(argc, argv);

  return base::LaunchUnitTests(
      argc, argv, base::Bind(&base::TestSuite::Run,
                             base::Unretained(&test_suite)));
}
