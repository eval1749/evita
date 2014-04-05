// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/abstract_dom_test.h"

namespace {

class TextEncoderTest : public dom::AbstractDomTest {
  protected: TextEncoderTest() {
  }
  public: virtual ~TextEncoderTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(TextEncoderTest);
};

TEST_F(TextEncoderTest, ctor) {
  // TODO(yosi) Invlaid label should thorw |TypeError|.
  EXPECT_SCRIPT_VALID("var encoder = new TextEncoder('foo');");
  EXPECT_SCRIPT_EQ("utf-8", "encoder.encoding");

  EXPECT_SCRIPT_VALID("var encoder1 = new TextEncoder();");
  EXPECT_SCRIPT_EQ("utf-8", "encoder.encoding");

  EXPECT_SCRIPT_VALID("var encoder2 = new TextEncoder('utf-8');");
  EXPECT_SCRIPT_EQ("utf-8", "encoder2.encoding");
}

TEST_F(TextEncoderTest, encode) {
  EXPECT_SCRIPT_VALID(
    "var encoder = new TextEncoder('utf-8');"
    "var buffer = encoder.encode('abc');");
  EXPECT_SCRIPT_EQ("3", "buffer.length");
  EXPECT_SCRIPT_EQ("97", "buffer[0]");
  EXPECT_SCRIPT_EQ("98", "buffer[1]");
  EXPECT_SCRIPT_EQ("99", "buffer[2]");
}

}  // namespace
