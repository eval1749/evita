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

TEST_F(ErrorsTest, TextDocumentError) {
  EXPECT_SCRIPT_VALID(
      "var doc = TextDocument.new('foo');"
      "var e1 = new TextDocumentError(doc, 'foo');");
  EXPECT_SCRIPT_TRUE("e1 instanceof Error");
  EXPECT_SCRIPT_TRUE("e1 instanceof TextDocumentError");
  EXPECT_SCRIPT_TRUE("e1.document === doc");
  EXPECT_SCRIPT_EQ("foo", "e1.message");
  EXPECT_SCRIPT_EQ("TextDocumentError", "e1.name");
}

TEST_F(ErrorsTest, TextDocumentNotReady) {
  EXPECT_SCRIPT_VALID(
      "var doc = TextDocument.new('foo');"
      "var e1 = new TextDocumentNotReady(doc, 'foo');");
  EXPECT_SCRIPT_TRUE("e1 instanceof Error");
  EXPECT_SCRIPT_TRUE("e1 instanceof TextDocumentError");
  EXPECT_SCRIPT_TRUE("e1 instanceof TextDocumentNotReady");
  EXPECT_SCRIPT_TRUE("e1.document === doc");
  EXPECT_SCRIPT_EQ("foo", "e1.message");
  EXPECT_SCRIPT_EQ("TextDocumentNotReady", "e1.name");
}

TEST_F(ErrorsTest, TextDocumentReadOnly) {
  EXPECT_SCRIPT_VALID(
      "var doc = TextDocument.new('foo');"
      "var e1 = new TextDocumentReadOnly(doc, 'foo');");
  EXPECT_SCRIPT_TRUE("e1 instanceof Error");
  EXPECT_SCRIPT_TRUE("e1 instanceof TextDocumentError");
  EXPECT_SCRIPT_TRUE("e1 instanceof TextDocumentReadOnly");
  EXPECT_SCRIPT_TRUE("e1.document === doc");
  EXPECT_SCRIPT_EQ("foo", "e1.message");
  EXPECT_SCRIPT_EQ("TextDocumentReadOnly", "e1.name");
}

}  // namespace
