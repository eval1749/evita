// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#include "gtest/gtest.h"

#include <string>

#include "base/basictypes.h"
#include "base/strings/string16.h"
#include "base/strings/utf_string_conversions.h"
#include "evita/dom/script_controller.h"
#include "evita/dom/view_delegate.h"

namespace dom {

class MockView : public dom::ViewDelegate {
  private: virtual void CreateEditorWindow(const EditorWindow*) override {}
  private: virtual void CreateTextWindow(const TextWindow*) override {}
  private: virtual void DestroyWindow(WidgetId) override {}
  private: virtual void RealizeWindow(WidgetId) override {}
  private: virtual void RegisterViewEventHandler(
      ViewEventHandler*) override {}
};

}  // namespace

namespace {

class ScriptTest : public ::testing::Test {
  private: dom::MockView mock_view_;
  // Note: ScriptController is a singleton.
  private: dom::ScriptController* script_controller_;

  protected: ScriptTest()
      : script_controller_(
          dom::ScriptController::StartForTesting(&mock_view_)) {
  }

  protected: std::string RunScript(const std::string& text) {
    auto eval_result = script_controller_->Evaluate(base::ASCIIToUTF16(text));
    return base::UTF16ToUTF8(eval_result.value);
  }

  DISALLOW_COPY_AND_ASSIGN(ScriptTest);
};

class EditorTest : public ScriptTest {
  protected: EditorTest() {
  }
  public: virtual ~EditorTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(EditorTest);
};

TEST_F(EditorTest, Version) {
  EXPECT_EQ("5.0", RunScript("editor.version"));
}

TEST_F(EditorTest, EditorWindow) {
  EXPECT_EQ("1", RunScript("(new EditorWindow()).id"));
  EXPECT_EQ("2", RunScript("(new EditorWindow()).id"));
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
