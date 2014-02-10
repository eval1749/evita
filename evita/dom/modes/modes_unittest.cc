// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/abstract_dom_test.h"

namespace {

class ModesTest : public dom::AbstractDomTest {
  public: ModesTest() = default;
  public: ~ModesTest() = default;

  DISALLOW_COPY_AND_ASSIGN(ModesTest);
};

TEST_F(ModesTest, chooseMode) {
  EXPECT_SCRIPT_VALID(
    "function testIt(mode_name) {"
    "  var doc = new Document('foo');"
    "  doc.properties.set('mode', mode_name);"
    "  return Mode.chooseMode(doc).name;"
    "}");
  EXPECT_SCRIPT_EQ("C++", "testIt('C++')");
  EXPECT_SCRIPT_EQ("Lisp", "testIt('Lisp')");
}

TEST_F(ModesTest, chooseModeByFileName) {
  EXPECT_SCRIPT_VALID(
      "function testIt(filename) {"
      "  return Mode.chooseModeByFileName(filename).name;"
      "}");
  EXPECT_SCRIPT_EQ("C++", "testIt('foo.cc')");
  EXPECT_SCRIPT_EQ("C++", "testIt('foo.cpp')");
  EXPECT_SCRIPT_EQ("C++", "testIt('foo.h')");
  EXPECT_SCRIPT_EQ("XML", "testIt('foo.html')");
  EXPECT_SCRIPT_EQ("Java", "testIt('foo.java')");
  EXPECT_SCRIPT_EQ("JavaScript", "testIt('foo.js')");
  EXPECT_SCRIPT_EQ("Plain", "testIt('foo.txt')");
  EXPECT_SCRIPT_EQ("XML", "testIt('foo.xml')");
  EXPECT_SCRIPT_TRUE("Mode.defaultMode.name == testIt('foo.unknown')");
  EXPECT_SCRIPT_TRUE("Mode.defaultMode.name == testIt('foo')");
}

TEST_F(ModesTest, ConfigMode) {
  EXPECT_SCRIPT_VALID("var mode = new ConfigMode();");
  EXPECT_SCRIPT_TRUE("ConfigMode.keymap instanceof Map");
  EXPECT_SCRIPT_TRUE("mode instanceof Mode");
  EXPECT_SCRIPT_TRUE("mode instanceof ConfigMode");
  EXPECT_SCRIPT_EQ("Config", "mode.name");
}

TEST_F(ModesTest, CxxMode) {
  EXPECT_SCRIPT_VALID("var mode = new CxxMode();");
  EXPECT_SCRIPT_TRUE("CxxMode.keymap instanceof Map");
  EXPECT_SCRIPT_TRUE("mode instanceof Mode");
  EXPECT_SCRIPT_TRUE("mode instanceof CxxMode");
  EXPECT_SCRIPT_EQ("C++", "mode.name");
}

TEST_F(ModesTest, HaskellMode) {
  EXPECT_SCRIPT_VALID("var mode = new HaskellMode();");
  EXPECT_SCRIPT_TRUE("HaskellMode.keymap instanceof Map");
  EXPECT_SCRIPT_TRUE("mode instanceof Mode");
  EXPECT_SCRIPT_TRUE("mode instanceof HaskellMode");
  EXPECT_SCRIPT_EQ("Haskell", "mode.name");
}

TEST_F(ModesTest, JavaMode) {
  EXPECT_SCRIPT_VALID("var mode = new JavaMode();");
  EXPECT_SCRIPT_TRUE("JavaMode.keymap instanceof Map");
  EXPECT_SCRIPT_TRUE("mode instanceof Mode");
  EXPECT_SCRIPT_TRUE("mode instanceof JavaMode");
  EXPECT_SCRIPT_EQ("Java", "mode.name");
}

TEST_F(ModesTest, JavaScriptMode) {
  EXPECT_SCRIPT_VALID("var mode = new JavaScriptMode();");
  EXPECT_SCRIPT_TRUE("JavaScriptMode.keymap instanceof Map");
  EXPECT_SCRIPT_TRUE("mode instanceof Mode");
  EXPECT_SCRIPT_TRUE("mode instanceof JavaScriptMode");
  EXPECT_SCRIPT_EQ("JavaScript", "mode.name");
}

TEST_F(ModesTest, LispMode) {
  EXPECT_SCRIPT_VALID("var mode = new LispMode();");
  EXPECT_SCRIPT_TRUE("LispMode.keymap instanceof Map");
  EXPECT_SCRIPT_TRUE("mode instanceof Mode");
  EXPECT_SCRIPT_TRUE("mode instanceof LispMode");
  EXPECT_SCRIPT_EQ("Lisp", "mode.name");
}

TEST_F(ModesTest, MasonMode) {
  EXPECT_SCRIPT_VALID("var mode = new MasonMode();");
  EXPECT_SCRIPT_TRUE("MasonMode.keymap instanceof Map");
  EXPECT_SCRIPT_TRUE("mode instanceof Mode");
  EXPECT_SCRIPT_TRUE("mode instanceof MasonMode");
  EXPECT_SCRIPT_EQ("Mason", "mode.name");
}

TEST_F(ModesTest, PerlMode) {
  EXPECT_SCRIPT_VALID("var mode = new PerlMode();");
  EXPECT_SCRIPT_TRUE("PerlMode.keymap instanceof Map");
  EXPECT_SCRIPT_TRUE("mode instanceof Mode");
  EXPECT_SCRIPT_TRUE("mode instanceof PerlMode");
  EXPECT_SCRIPT_EQ("Perl", "mode.name");
}

TEST_F(ModesTest, PlainTextMode) {
  EXPECT_SCRIPT_VALID("var mode = new PlainTextMode();");
  EXPECT_SCRIPT_TRUE("PlainTextMode.keymap instanceof Map");
  EXPECT_SCRIPT_TRUE("mode instanceof Mode");
  EXPECT_SCRIPT_TRUE("mode instanceof PlainTextMode");
  EXPECT_SCRIPT_EQ("Plain", "mode.name");
}

TEST_F(ModesTest, PythonMode) {
  EXPECT_SCRIPT_VALID("var mode = new PythonMode();");
  EXPECT_SCRIPT_TRUE("PythonMode.keymap instanceof Map");
  EXPECT_SCRIPT_TRUE("mode instanceof Mode");
  EXPECT_SCRIPT_TRUE("mode instanceof PythonMode");
  EXPECT_SCRIPT_EQ("Python", "mode.name");
}

TEST_F(ModesTest, XmlMode) {
  EXPECT_SCRIPT_VALID("var mode = new XmlMode();");
  EXPECT_SCRIPT_TRUE("XmlMode.keymap instanceof Map");
  EXPECT_SCRIPT_TRUE("mode instanceof Mode");
  EXPECT_SCRIPT_TRUE("mode instanceof XmlMode");
  EXPECT_SCRIPT_EQ("XML", "mode.name");
}

}  // namespace
