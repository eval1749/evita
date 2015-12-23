// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include <string>

#include "base/basictypes.h"
#include "gmock/gmock.h"
#include "evita/dom/testing/abstract_dom_test.h"
#include "evita/dom/mock_io_delegate.h"
#include "evita/dom/mock_view_impl.h"
#include "evita/dom/script_host.h"

namespace {

using ::testing::_;

class EditorTest : public dom::AbstractDomTest {
 public:
  ~EditorTest() override = default;

 protected:
  EditorTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(EditorTest);
};

TEST_F(EditorTest, checkSpelling) {
  EXPECT_SCRIPT_VALID(
      "var result;"
      "function checkSpelling(word) {"
      "  Editor.checkSpelling(word).then(function(x) { result = x; });"
      "}");
  mock_io_delegate()->set_check_spelling_result(true);
  EXPECT_SCRIPT_VALID("checkSpelling('word')");
  EXPECT_SCRIPT_TRUE("result");

  mock_io_delegate()->set_check_spelling_result(false);
  EXPECT_SCRIPT_VALID("checkSpelling('wrod')");
  EXPECT_SCRIPT_FALSE("result");
}

TEST_F(EditorTest, getFileNameForLoad) {
  EXPECT_CALL(*mock_view_impl(), CreateEditorWindow(_));
  EXPECT_SCRIPT_VALID(
      "var file_name;"
      "var window = new EditorWindow();"
      "Editor.getFileNameForLoad(window, 'dir').then(function(x) {"
      "  file_name = x;"
      "});");
  EXPECT_SCRIPT_EQ("dir/foo.bar", "file_name");
}

TEST_F(EditorTest, getFileNameForSave) {
  EXPECT_CALL(*mock_view_impl(), CreateEditorWindow(_));
  EXPECT_SCRIPT_VALID(
      "var file_name;"
      "var window = new EditorWindow();"
      "Editor.getFileNameForSave(window, 'dir').then(function(x) {"
      "  file_name = x;"
      "});");
  EXPECT_SCRIPT_EQ("dir/foo.bar", "file_name");
}

TEST_F(EditorTest, getSpellingSuggestions) {
  EXPECT_SCRIPT_VALID(
      "var result;"
      "function getSpellingSuggestions(word) {"
      "  Editor.getSpellingSuggestions(word).then(function(x) { result = x; });"
      "}");
  mock_io_delegate()->set_spelling_suggestions({L"word", L"sword"});
  EXPECT_SCRIPT_VALID("getSpellingSuggestions('word')");
  EXPECT_SCRIPT_EQ("word, sword", "result.join(', ')");
}

TEST_F(EditorTest, localizeText) {
  EXPECT_SCRIPT_EQ("This is a pen",
                   "Editor.localizeText('This is a __item__', {item: 'pen'})");
}

TEST_F(EditorTest, messageBox) {
  EXPECT_CALL(*mock_view_impl(), CreateEditorWindow(_));
  EXPECT_SCRIPT_VALID(
      "var response_code;"
      "var window = new EditorWindow();"
      "Editor.messageBox(window, 'message', 3, 'title')"
      "    .then(function(x) {"
      "       response_code = x;"
      "    });");
  EXPECT_SCRIPT_EQ("3", "response_code");
}

TEST_F(EditorTest, runScript) {
  EXPECT_SCRIPT_EQ("1", "Editor.runScript('1').value");
  EXPECT_SCRIPT_EQ("ReferenceError: foo is not defined,0,1",
                   "var result = Editor.runScript('foo');"
                   "[result.exception, result.start, result.end]");
}

TEST_F(EditorTest, version) {
  EXPECT_SCRIPT_EQ("5.0", "Editor.version");
}

}  // namespace

#include "base/bind.h"
#pragma warning(push)
#include "base/test/launcher/unit_test_launcher.h"
#pragma warning(disable : 4625)
#include "base/test/test_suite.h"
#pragma warning(pop)

int main(int argc, char** argv) {
  base::TestSuite test_suite(argc, argv);

  return base::LaunchUnitTests(
      argc, argv,
      base::Bind(&base::TestSuite::Run, base::Unretained(&test_suite)));
}
