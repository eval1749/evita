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

TEST_F(ModesTest, ConfigMode) {
  EXPECT_SCRIPT_VALID(
    "var doc = new Document('foo');"
    "var mode = new ConfigMode(doc);");
  EXPECT_SCRIPT_TRUE("ConfigMode.keymap instanceof Map");
  EXPECT_SCRIPT_TRUE("mode instanceof Mode");
  EXPECT_SCRIPT_TRUE("mode instanceof ConfigMode");
  EXPECT_SCRIPT_TRUE("mode.document === doc");
  EXPECT_SCRIPT_EQ("Config", "mode.name");
}

TEST_F(ModesTest, CxxMode) {
  EXPECT_SCRIPT_VALID(
    "var doc = new Document('foo');"
    "var mode = new CxxMode(doc);");
  EXPECT_SCRIPT_TRUE("CxxMode.keymap instanceof Map");
  EXPECT_SCRIPT_TRUE("mode instanceof Mode");
  EXPECT_SCRIPT_TRUE("mode instanceof CxxMode");
  EXPECT_SCRIPT_TRUE("mode.document === doc");
  EXPECT_SCRIPT_EQ("C++", "mode.name");
}

TEST_F(ModesTest, HaskellMode) {
  EXPECT_SCRIPT_VALID(
    "var doc = new Document('foo');"
    "var mode = new HaskellMode(doc);");
  EXPECT_SCRIPT_TRUE("HaskellMode.keymap instanceof Map");
  EXPECT_SCRIPT_TRUE("mode instanceof Mode");
  EXPECT_SCRIPT_TRUE("mode instanceof HaskellMode");
  EXPECT_SCRIPT_TRUE("mode.document === doc");
  EXPECT_SCRIPT_EQ("Haskell", "mode.name");
}

TEST_F(ModesTest, JavaMode) {
  EXPECT_SCRIPT_VALID(
    "var doc = new Document('foo');"
    "var mode = new JavaMode(doc);");
  EXPECT_SCRIPT_TRUE("JavaMode.keymap instanceof Map");
  EXPECT_SCRIPT_TRUE("mode instanceof Mode");
  EXPECT_SCRIPT_TRUE("mode instanceof JavaMode");
  EXPECT_SCRIPT_TRUE("mode.document === doc");
  EXPECT_SCRIPT_EQ("Java", "mode.name");
}

TEST_F(ModesTest, LispMode) {
  EXPECT_SCRIPT_VALID(
    "var doc = new Document('foo');"
    "var mode = new LispMode(doc);");
  EXPECT_SCRIPT_TRUE("LispMode.keymap instanceof Map");
  EXPECT_SCRIPT_TRUE("mode instanceof Mode");
  EXPECT_SCRIPT_TRUE("mode instanceof LispMode");
  EXPECT_SCRIPT_TRUE("mode.document === doc");
  EXPECT_SCRIPT_EQ("Lisp", "mode.name");
}

TEST_F(ModesTest, MasonMode) {
  EXPECT_SCRIPT_VALID(
    "var doc = new Document('foo');"
    "var mode = new MasonMode(doc);");
  EXPECT_SCRIPT_TRUE("MasonMode.keymap instanceof Map");
  EXPECT_SCRIPT_TRUE("mode instanceof Mode");
  EXPECT_SCRIPT_TRUE("mode instanceof MasonMode");
  EXPECT_SCRIPT_TRUE("mode.document === doc");
  EXPECT_SCRIPT_EQ("Mason", "mode.name");
}

}  // namespace
