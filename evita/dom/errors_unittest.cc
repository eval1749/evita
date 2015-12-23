// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/testing/abstract_dom_test.h"

namespace {

class ErrorsTest : public dom::AbstractDomTest {
 public:
  ~ErrorsTest() override = default;

 protected:
  ErrorsTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(ErrorsTest);
};

TEST_F(ErrorsTest, DocumentError) {
  EXPECT_SCRIPT_VALID(
      "var doc = Document.new('foo');"
      "var e1 = new DocumentError(doc, 'foo');");
  EXPECT_SCRIPT_TRUE("e1 instanceof Error");
  EXPECT_SCRIPT_TRUE("e1 instanceof DocumentError");
  EXPECT_SCRIPT_TRUE("e1.document === doc");
  EXPECT_SCRIPT_EQ("foo", "e1.message");
  EXPECT_SCRIPT_EQ("DocumentError", "e1.name");
}

TEST_F(ErrorsTest, DocumentNotReady) {
  EXPECT_SCRIPT_VALID(
      "var doc = Document.new('foo');"
      "var e1 = new DocumentNotReady(doc, 'foo');");
  EXPECT_SCRIPT_TRUE("e1 instanceof Error");
  EXPECT_SCRIPT_TRUE("e1 instanceof DocumentError");
  EXPECT_SCRIPT_TRUE("e1 instanceof DocumentNotReady");
  EXPECT_SCRIPT_TRUE("e1.document === doc");
  EXPECT_SCRIPT_EQ("foo", "e1.message");
  EXPECT_SCRIPT_EQ("DocumentNotReady", "e1.name");
}

TEST_F(ErrorsTest, DocumentReadOnly) {
  EXPECT_SCRIPT_VALID(
      "var doc = Document.new('foo');"
      "var e1 = new DocumentReadOnly(doc, 'foo');");
  EXPECT_SCRIPT_TRUE("e1 instanceof Error");
  EXPECT_SCRIPT_TRUE("e1 instanceof DocumentError");
  EXPECT_SCRIPT_TRUE("e1 instanceof DocumentReadOnly");
  EXPECT_SCRIPT_TRUE("e1.document === doc");
  EXPECT_SCRIPT_EQ("foo", "e1.message");
  EXPECT_SCRIPT_EQ("DocumentReadOnly", "e1.name");
}

}  // namespace
