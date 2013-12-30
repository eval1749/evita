// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#include "gtest/gtest.h"

#include <string>

#include "base/basictypes.h"
#include "base/strings/string16.h"
#include "base/strings/utf_string_conversions.h"
#include "evita/dom/script_controller.h"
#include "evita/editor/application.h"

namespace {

class ScriptTest : public ::testing::Test {
  private: dom::EvaluateResult eval_result_;

  protected: ScriptTest() {
  }

  protected: std::string RunScript(const std::string& text) {
    dom::ScriptController::User::instance()->Evaluate(
        base::ASCIIToUTF16(text),
        base::Bind(&ScriptTest::DidEvaluate, base::Unretained(this)));
    Application::instance()->Run();
    return base::UTF16ToUTF8(eval_result_.value);
  }

  private: void DidEvaluate(dom::EvaluateResult result) {
    eval_result_ = result;
    Application::instance()->QuitForTest();
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
  EXPECT_EQ("3", RunScript("1 + 2"));
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
