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

  public: base::string16 DecodeStream(encodings::Decoder* decoder,
                                const std::vector<uint8_t> bytes) {
    auto const result = decoder->Decode(bytes.data(), bytes.size(), true);
    return result.left ? result.right : L"EncodingError";
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

TEST_F(EncodingsTest, EucJpDecoder) {
  auto const decoder = Encodings::instance()->GetDecoder(L"euc-jp");
  EXPECT_EQ(base::string16(L"ax"),
            Decode(decoder, std::vector<uint8_t> {0x61, 0x78})) <<
    "ASCII";
  EXPECT_EQ(base::string16(L"\u611B"),
            Decode(decoder, std::vector<uint8_t> {0xB0, 0xA6})) <<
    "Kanji character";

  EXPECT_EQ(base::string16(L"\uFF75"),
            Decode(decoder, std::vector<uint8_t> {0x8E, 0xB5})) <<
    "Half-width katakana character";
}

TEST_F(EncodingsTest, EucJpEncoder) {
  auto const encoder = Encodings::instance()->GetEncoder(L"euc-jp");
  EXPECT_EQ((std::vector<uint8_t> {0x61, 0x78}),
            Encode(encoder, L"ax")) << "ASCII";
  EXPECT_EQ((std::vector<uint8_t> {0xB0, 0xA6}),
            Encode(encoder, base::string16(L"\u611B"))) << "Kanji";
  EXPECT_EQ((std::vector<uint8_t> {0x00, 0x01}),
            Encode(encoder, base::string16(L"\u0100"))) <<
    "No translation";

  // Half-width Katakana
  EXPECT_EQ((std::vector<uint8_t> {0x8E, 0xB5}),
            Encode(encoder, base::string16(L"\uFF75"))) <<
    "Half-width Katakan";

  // EUC-JP special code mapping
  EXPECT_EQ((std::vector<uint8_t> {0x5C}),
            Encode(encoder, base::string16(L"\u00A5"))) << "backslash";
  EXPECT_EQ((std::vector<uint8_t> {0x7E}),
            Encode(encoder, base::string16(L"\u203E"))) << "tilda";
}

TEST_F(EncodingsTest, ShiftJisDecoder) {
  auto const decoder = Encodings::instance()->GetDecoder(L"shift_jis");
  EXPECT_EQ(base::string16(L"ax"),
            Decode(decoder, std::vector<uint8_t> {0x61, 0x78}));
  EXPECT_EQ(base::string16(L"\u611B"),
            Decode(decoder, std::vector<uint8_t> {0x88, 0xA4}));
}

TEST_F(EncodingsTest, ShiftJisEncoder) {
  auto const encoder = Encodings::instance()->GetEncoder(L"shift_jis");
  EXPECT_EQ((std::vector<uint8_t> {0x61, 0x78}),
            Encode(encoder, L"ax"));
  EXPECT_EQ((std::vector<uint8_t> {0x88, 0xA4}),
            Encode(encoder, base::string16(L"\u611B")));
  EXPECT_EQ((std::vector<uint8_t> {0x00, 0x01}),
            Encode(encoder, base::string16(L"\u0100")));
}

TEST_F(EncodingsTest, Utf8Decoder) {
  auto const decoder = Encodings::instance()->GetDecoder(L"utf-8");
  EXPECT_EQ(base::string16(L"ax"),
            Decode(decoder, std::vector<uint8_t> {0x61, 0x78}));
  EXPECT_EQ(base::string16(L"\u611B"),
            Decode(decoder, std::vector<uint8_t> {0xE6, 0x84, 0x9B}));
  base::string16 surrogate_pair(2, 0);
  surrogate_pair[0] = 0xD842;  // U+20BB7
  surrogate_pair[1] = 0xDFB7;
  EXPECT_EQ(surrogate_pair,
            Decode(decoder, std::vector<uint8_t> {0xF0, 0xA0, 0xAE, 0xB7})) <<
      "UTF-16 surrogate D800..DBFF, DC00..DFFF";
  EXPECT_EQ(L"EncodingError",
            DecodeStream(decoder, std::vector<uint8_t>{0x41, 0xA9, 0x42})) <<
      "Bad UTF-8 byte stream, it contains 0xA9.";
}

TEST_F(EncodingsTest, Utf8Encoder) {
  auto const encoder = Encodings::instance()->GetEncoder(L"utf-8");
  EXPECT_EQ((std::vector<uint8_t> {0x61, 0x78}),
            Encode(encoder, L"ax"));
  EXPECT_EQ((std::vector<uint8_t> {0xE6, 0x84, 0x9B}),
            Encode(encoder, base::string16(L"\u611B")));
}

}  // namespace
