// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/testing/abstract_dom_test.h"

namespace dom {

class TextEncoderTest : public AbstractDomTest {
 public:
  ~TextEncoderTest() override = default;

 protected:
  TextEncoderTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(TextEncoderTest);
};

TEST_F(TextEncoderTest, ctor) {
  EXPECT_SCRIPT_EQ(
      "Error: Failed to construct 'TextEncoder': No such encoding 'foo'",
      "var decoder = new TextEncoder('foo');");

  EXPECT_SCRIPT_VALID("var encoder1 = new TextEncoder();");
  EXPECT_SCRIPT_EQ("utf-8", "encoder1.encoding");

  EXPECT_SCRIPT_VALID("var encoder2 = new TextEncoder('utf-8');");
  EXPECT_SCRIPT_EQ("utf-8", "encoder2.encoding");
}

TEST_F(TextEncoderTest, encode_utf8_ascii) {
  EXPECT_SCRIPT_VALID(
      "var encoder = new TextEncoder('utf-8');"
      "var buffer = encoder.encode('abc');");
  EXPECT_SCRIPT_EQ("3", "buffer.length");
  EXPECT_SCRIPT_EQ("97", "buffer[0]");
  EXPECT_SCRIPT_EQ("98", "buffer[1]");
  EXPECT_SCRIPT_EQ("99", "buffer[2]");
}

TEST_F(TextEncoderTest, encode_utf8_kanji) {
  EXPECT_SCRIPT_VALID(
      "var encoder = new TextEncoder('utf-8');"
      "var buffer = encoder.encode('\\u611B\\u60C5');");
  EXPECT_SCRIPT_EQ("6", "buffer.length");
  EXPECT_SCRIPT_EQ("230", "buffer[0]");
  EXPECT_SCRIPT_EQ("132", "buffer[1]");
  EXPECT_SCRIPT_EQ("155", "buffer[2]");
  EXPECT_SCRIPT_EQ("230", "buffer[3]");
  EXPECT_SCRIPT_EQ("131", "buffer[4]");
  EXPECT_SCRIPT_EQ("133", "buffer[5]");
}

}  // namespace dom
