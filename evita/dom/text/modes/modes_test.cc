// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/testing/abstract_dom_test.h"

namespace dom {

class ModesTest : public AbstractDomTest {
 protected:
  ModesTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(ModesTest);
};

TEST_F(ModesTest, chooseMode) {
  EXPECT_SCRIPT_VALID(
      "function testIt(mode_name) {"
      "  var doc = TextDocument.new('foo');"
      "  doc.properties.set('mode', mode_name);"
      "  return Mode.chooseMode(doc).name;"
      "}");
  EXPECT_SCRIPT_EQ("C++", "testIt('C++')");
  EXPECT_SCRIPT_EQ("Lisp", "testIt('Lisp')");
}

TEST_F(ModesTest, chooseModeByFileName) {
  EXPECT_SCRIPT_VALID(
      "function testIt(file_name) {"
      "  return Mode.chooseModeByFileName(file_name).name;"
      "}");
  EXPECT_SCRIPT_EQ("C++", "testIt('foo.cc')");
  EXPECT_SCRIPT_EQ("C++", "testIt('foo.cpp')");
  EXPECT_SCRIPT_EQ("C++", "testIt('foo.h')");
  EXPECT_SCRIPT_EQ("HTML", "testIt('foo.htm')");
  EXPECT_SCRIPT_EQ("HTML", "testIt('foo.html')");
  EXPECT_SCRIPT_EQ("Java", "testIt('foo.java')");
  EXPECT_SCRIPT_EQ("JavaScript", "testIt('foo.js')");
  EXPECT_SCRIPT_EQ("Plain", "testIt('foo.txt')");
  EXPECT_SCRIPT_EQ("HTML", "testIt('foo.xhtml')");
  EXPECT_SCRIPT_EQ("XML", "testIt('foo.xml')");
  EXPECT_SCRIPT_TRUE("Mode.defaultMode.name == testIt('foo.unknown')");
  EXPECT_SCRIPT_TRUE("Mode.defaultMode.name == testIt('foo')");
}

TEST_F(ModesTest, ConfigMode) {
  EXPECT_SCRIPT_VALID("var mode = Mode.chooseModeByFileName('foo.mk');");
  EXPECT_SCRIPT_TRUE("mode.constructor.keymap instanceof Map");
  EXPECT_SCRIPT_TRUE("mode instanceof Mode");
  EXPECT_SCRIPT_EQ("Config", "mode.name");
}

TEST_F(ModesTest, CxxMode) {
  EXPECT_SCRIPT_VALID("var mode = Mode.chooseModeByFileName('foo.cc');");
  EXPECT_SCRIPT_TRUE("mode.constructor.keymap instanceof Map");
  EXPECT_SCRIPT_TRUE("mode instanceof Mode");
  EXPECT_SCRIPT_EQ("C++", "mode.name");
}

TEST_F(ModesTest, HaskellMode) {
  EXPECT_SCRIPT_VALID("var mode = Mode.chooseModeByFileName('foo.hs');");
  EXPECT_SCRIPT_TRUE("mode.constructor.keymap instanceof Map");
  EXPECT_SCRIPT_TRUE("mode instanceof Mode");
  EXPECT_SCRIPT_EQ("Haskell", "mode.name");
}

TEST_F(ModesTest, JavaMode) {
  EXPECT_SCRIPT_VALID("var mode = Mode.chooseModeByFileName('foo.java');");
  EXPECT_SCRIPT_TRUE("mode.constructor.keymap instanceof Map");
  EXPECT_SCRIPT_TRUE("mode instanceof Mode");
  EXPECT_SCRIPT_EQ("Java", "mode.name");
}

TEST_F(ModesTest, JavaScriptMode) {
  EXPECT_SCRIPT_VALID("var mode = Mode.chooseModeByFileName('foo.js');");
  EXPECT_SCRIPT_TRUE("mode.constructor.keymap instanceof Map");
  EXPECT_SCRIPT_TRUE("mode instanceof Mode");
  EXPECT_SCRIPT_EQ("JavaScript", "mode.name");
}

TEST_F(ModesTest, LispMode) {
  EXPECT_SCRIPT_VALID("var mode = Mode.chooseModeByFileName('foo.l');");
  EXPECT_SCRIPT_TRUE("mode.constructor.keymap instanceof Map");
  EXPECT_SCRIPT_TRUE("mode instanceof Mode");
  EXPECT_SCRIPT_EQ("Lisp", "mode.name");
}

TEST_F(ModesTest, MasonMode) {
  EXPECT_SCRIPT_VALID("var mode = Mode.chooseModeByFileName('foo.m');");
  EXPECT_SCRIPT_TRUE("mode.constructor.keymap instanceof Map");
  EXPECT_SCRIPT_TRUE("mode instanceof Mode");
  EXPECT_SCRIPT_EQ("Mason", "mode.name");
}

TEST_F(ModesTest, PerlMode) {
  EXPECT_SCRIPT_VALID("var mode = Mode.chooseModeByFileName('foo.pl');");
  EXPECT_SCRIPT_TRUE("mode.constructor.keymap instanceof Map");
  EXPECT_SCRIPT_TRUE("mode instanceof Mode");
  EXPECT_SCRIPT_EQ("Perl", "mode.name");
}

TEST_F(ModesTest, PlainTextMode) {
  EXPECT_SCRIPT_VALID("var mode = Mode.chooseModeByFileName('foo.txt');");
  EXPECT_SCRIPT_TRUE("mode.constructor.keymap instanceof Map");
  EXPECT_SCRIPT_TRUE("mode instanceof Mode");
  EXPECT_SCRIPT_EQ("Plain", "mode.name");
}

TEST_F(ModesTest, PythonMode) {
  EXPECT_SCRIPT_VALID("var mode = Mode.chooseModeByFileName('foo.py');");
  EXPECT_SCRIPT_TRUE("mode.constructor.keymap instanceof Map");
  EXPECT_SCRIPT_TRUE("mode instanceof Mode");
  EXPECT_SCRIPT_EQ("Python", "mode.name");
}

TEST_F(ModesTest, XmlMode) {
  EXPECT_SCRIPT_VALID("var mode = Mode.chooseModeByFileName('foo.xml');");
  EXPECT_SCRIPT_TRUE("mode.constructor.keymap instanceof Map");
  EXPECT_SCRIPT_TRUE("mode instanceof Mode");
  EXPECT_SCRIPT_EQ("XML", "mode.name");
}

}  // namespace dom
