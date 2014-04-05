// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#pragma warning(push)
#pragma warning(disable: 4365 4625 4626 4826)
#include "gtest/gtest.h"
#pragma warning(pop)

#include "evita/text/encodings/decoder.h"
#include "evita/text/encodings/encodings.h"
#include "evita/text/encodings/encoder.h"

namespace {

using encodings::Encodings;

class EncodingsTest : public ::testing::Test {
  protected: EncodingsTest() {
  }
  public: virtual ~EncodingsTest() {
  }

  public: base::string16 Decode(encodings::Decoder* decoder,
                                const std::vector<uint8_t> bytes) {
    auto const result = decoder->Decode(bytes.data(), bytes.size(), false);
    return result.left ? result.right : L"";
  }

  public: std::vector<uint8_t> Encode(encodings::Encoder* encoder,
                                      const base::string16& string) {
    auto const result = encoder->Encode(string, false);
    if (!result.left)
      return result.right;
    return std::vector<uint8_t> {
        static_cast<uint8_t>(result.left & 0xFF),
        static_cast<uint8_t>((result.left >> 8) & 0xFF),
    };
  }

  DISALLOW_COPY_AND_ASSIGN(EncodingsTest);
};

TEST_F(EncodingsTest, Utf8Decoder) {
  auto const decoder = Encodings::instance()->GetDecoder(L"utf-8");
  EXPECT_EQ(base::string16(L"ax"),
            Decode(decoder, std::vector<uint8_t> { 0x61, 0x78 }));
  EXPECT_EQ(base::string16(L"\u611B"),
            Decode(decoder, std::vector<uint8_t> { 0xE6, 0x84, 0x9B }));
}

TEST_F(EncodingsTest, Utf8Encoder) {
  auto const encoder = Encodings::instance()->GetEncoder(L"utf-8");
  EXPECT_EQ((std::vector<uint8_t> { 0x61, 0x78 }),
            Encode(encoder, L"ax"));
  EXPECT_EQ((std::vector<uint8_t> { 0xE6, 0x84, 0x9B }),
            Encode(encoder, base::string16(L"\u611B")));
}

}  // namespace
