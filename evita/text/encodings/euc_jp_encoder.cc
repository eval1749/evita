// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <windows.h>

#include "evita/text/encodings/euc_jp_encoder.h"

#include "base/logging.h"

namespace encodings {

//////////////////////////////////////////////////////////////////////
//
// EucJpEncoder::Private
//
class EucJpEncoder::Private {
  public: Private();
  public: ~Private();

  public: common::Either<base::char16, std::vector<uint8_t>>
      Encode(const base::string16& string, bool is_stream);

  private: common::Either<base::char16, std::vector<uint8_t>>
      Error(int code_point);

  private: common::Either<base::char16, std::vector<uint8_t>>
      Succeeded(std::vector<uint8_t> bytes);

  DISALLOW_COPY_AND_ASSIGN(Private);
};  // EucJpEncoder

EucJpEncoder::Private::Private() {
}

EucJpEncoder::Private::~Private() {
}

static void ConvertShiftJisToJis(uint8_t* bytes) {
  auto const adjust = bytes[1] < 159;
  auto const row = bytes[0] < 160 ? 112 : 176;
  auto const cell = adjust ? (bytes[1] > 127 ? 32 : 11) : 126;
  bytes[0] = static_cast<uint8_t>(((bytes[0] - row) << 1) - adjust);
  bytes[1] = static_cast<uint8_t>(bytes[1] - cell);
}

// Because of Window doesn't support EUC-JP, event if it defines code pages
// 50932, 51932 (EUC-JP 2012), by default, we convert EUC-JP to Shift_JIS
// then convert to Unicode.
common::Either<base::char16, std::vector<uint8_t>>
    EucJpEncoder::Private::Encode(const base::string16& string, bool) {
  if (string.empty())
    return Succeeded(std::vector<uint8_t>());
  std::vector<uint8_t> output(string.size() * 2);
  output.resize(0);
  for (auto const code_point : string) {
    if (code_point == 0xA5) {
      output.push_back(static_cast<uint8_t>(0x5C));
      continue;
    }
    if (code_point == 0x203E) {
      output.push_back(static_cast<uint8_t>(0x7E));
      continue;
    }
    if (code_point >= 0xFF61 && code_point <= 0xFF9F) {
      output.push_back(static_cast<uint8_t>(0x8E));
      output.push_back(static_cast<uint8_t>(code_point - 0xFF61 + 0xA1));
      continue;
    }
    auto const kShiftJisCodePage = 932;
    BOOL used_default_char = FALSE;
    uint8_t bytes[2];
    auto const num_bytes = static_cast<size_t>(::WideCharToMultiByte(
        kShiftJisCodePage, 0, &code_point, 1, reinterpret_cast<char*>(bytes),
        2, nullptr, &used_default_char));
    if (!used_default_char) {
      if (num_bytes == 1) {
        output.push_back(bytes[0]);
        continue;
      }
      if (num_bytes == 2) {
        ConvertShiftJisToJis(bytes);
        output.push_back(static_cast<uint8_t>(bytes[0] | 0x80));
        output.push_back(static_cast<uint8_t>(bytes[1] | 0x80));
        continue;
      }
    }
    return Error(code_point);
  }
  output.resize(output.size());
  return Succeeded(output);
}

common::Either<base::char16, std::vector<uint8_t>>
    EucJpEncoder::Private::Error(int code_point) {
  return common::make_either(static_cast<base::char16>(code_point),
                             std::vector<uint8_t>());
}

common::Either<base::char16, std::vector<uint8_t>>
    EucJpEncoder::Private::Succeeded(std::vector<uint8_t> bytes) {
  return common::make_either(static_cast<base::char16>(0), bytes);
}

//////////////////////////////////////////////////////////////////////
//
// EucJpEncoder
//
EucJpEncoder::EucJpEncoder() : private_(new Private()) {
}

EucJpEncoder::~EucJpEncoder() {
}

// encoding::Encoder
const base::string16& EucJpEncoder::name() const {
  CR_DEFINE_STATIC_LOCAL(base::string16, name_, (static_name()));
  return name_;
}

common::Either<base::char16, std::vector<uint8_t>> EucJpEncoder::Encode(
    const base::string16& string, bool is_stream) {
  return private_->Encode(string, is_stream);
}

}  // namespace encodings
