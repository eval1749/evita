// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/abstract_dom_test.h"

namespace {

class TextDecoderTest : public dom::AbstractDomTest {
  protected: TextDecoderTest() {
  }
  public: virtual ~TextDecoderTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(TextDecoderTest);
};

TEST_F(TextDecoderTest, ctor) {
  EXPECT_SCRIPT_EQ(
      "Error: No such encoding 'foo'",
      "var decoder = new TextDecoder('foo');");

  EXPECT_SCRIPT_VALID("var decoder1 = new TextDecoder();");
  EXPECT_SCRIPT_EQ("utf-8", "decoder1.encoding");

  EXPECT_SCRIPT_VALID("var decoder2 = new TextDecoder('utf-8');");
  EXPECT_SCRIPT_EQ("utf-8", "decoder2.encoding");

  EXPECT_SCRIPT_VALID("var decoder3 = new TextDecoder('utf-8', {fatal:true});");
  EXPECT_SCRIPT_EQ("utf-8", "decoder3.encoding");

  EXPECT_SCRIPT_VALID("var decoder4 = new TextDecoder('utf-8', {fatal:false});");
  EXPECT_SCRIPT_EQ("utf-8", "decoder4.encoding");
}

TEST_F(TextDecoderTest, decode) {
  EXPECT_SCRIPT_VALID(
    "var decoder = new TextDecoder('utf-8');"
    "var buffer = new Uint8Array(new ArrayBuffer(3));"
    "buffer[0] = 'abc'.charCodeAt(0);"
    "buffer[1] = 'abc'.charCodeAt(1);"
    "buffer[2] = 'abc'.charCodeAt(2);");
  EXPECT_SCRIPT_EQ("abc", "decoder.decode(buffer);");
  EXPECT_SCRIPT_EQ("", "decoder.decode();");
}

}  // namespace
