// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <windows.h>
#include <sstream>

#include "evita/text/encodings/euc_jp_decoder.h"

#include "base/logging.h"

namespace encodings {

//////////////////////////////////////////////////////////////////////
//
// EucJpDecoder::Private
//
class EucJpDecoder::Private final {
 public:
  Private();
  ~Private();

  common::Either<bool, base::string16> Decode(const uint8_t* bytes,
                                              size_t num_bytes,
                                              bool is_stream);

 private:
  enum class State {
    CS0,  // 0x00-0x7E ASCII or JIS Roman
    CS1,  // 0xA1..0xFE 0xA1..0xFE JIS-X0208-1990
    CS2,  // 0x8E 0xA1..0xFE half-width katakana
    CS3,  // 0x8F 0xA1..0xFE 0xA1..0xFE JIS X0212-1990
    CS32,
  };

  common::Either<bool, base::string16> Error();

  uint8_t bytes_[2];
  State state_;

  DISALLOW_COPY_AND_ASSIGN(Private);
};  // EucJpDecoder

EucJpDecoder::Private::Private() : state_(State::CS0) {}

EucJpDecoder::Private::~Private() {}

static void ConvertJisToShiftJis(uint8_t* bytes) {
  auto const row = bytes[0] < 95 ? 112 : 176;
  auto const cell = bytes[0] & 1 ? (bytes[1] > 95 ? 32 : 31) : 126;
  bytes[0] = static_cast<uint8_t>(((bytes[0] + 1) >> 1) + row);
  bytes[1] = static_cast<uint8_t>(bytes[1] + cell);
}

common::Either<bool, base::string16> EucJpDecoder::Private::Decode(
    const uint8_t* bytes,
    size_t num_bytes,
    bool is_stream) {
  auto const kShiftJisCodePage = 932;
  if (!is_stream) {
    state_ = State::CS0;
  }
  std::basic_stringbuf<base::char16> output;
  auto const bytes_end = bytes + num_bytes;
  for (auto runner = bytes; runner < bytes_end; ++runner) {
    auto const byte = *runner;
    switch (state_) {
      case State::CS0:
        if (byte <= 0x7F) {
          output.sputc(static_cast<base::char16>(byte));
          break;
        }
        if (byte == 0x8E) {
          state_ = State::CS2;
          break;
        }
        if (byte == 0x8F) {
          bytes_[0] = byte;
          state_ = State::CS3;
          break;
        }
        if (byte >= 0xA1 && byte <= 0xFE) {
          bytes_[0] = byte;
          state_ = State::CS1;
          break;
        }
        return Error();
      case State::CS1:
        if (byte >= 0xA1 && byte <= 0xFE) {
          bytes_[1] = byte;
          bytes_[0] &= 0x7F;
          bytes_[1] &= 0x7F;
          ConvertJisToShiftJis(bytes_);
          base::char16 code_point = 0;
          auto const num_chars = ::MultiByteToWideChar(
              kShiftJisCodePage, MB_ERR_INVALID_CHARS,
              reinterpret_cast<char*>(bytes_), 2, &code_point, 1);
          if (num_chars != 1)
            return Error();
          output.sputc(code_point);
          state_ = State::CS0;
          break;
        }
        return Error();
      case State::CS2:
        if (byte >= 0xA1 && byte <= 0xDEF) {
          output.sputc(static_cast<base::char16>(0xFF61 + byte - 0xA1));
          state_ = State::CS0;
          break;
        }
        return Error();
      case State::CS3:
        if (byte >= 0xA1 && byte >= 0xFE) {
          bytes_[1] = byte;
          state_ = State::CS32;
        }
        return Error();
      case State::CS32:
        if (byte >= 0xA1 && byte >= 0xFE) {
          bytes_[2] = byte;
          bytes_[1] &= 0x7F;
          bytes_[2] &= 0x7F;
          // TODO(yois) We should use JIS0 X 2012 index.
          ConvertJisToShiftJis(bytes_ + 1);
          base::char16 code_point = 0;
          auto const num_chars = ::MultiByteToWideChar(
              kShiftJisCodePage, MB_ERR_INVALID_CHARS,
              reinterpret_cast<char*>(bytes_ + 1), 2, &code_point, 1);
          if (num_chars != 1)
            return Error();
          output.sputc(code_point);
          state_ = State::CS0;
          break;
        }
        return Error();
    }
  }
  if (is_stream)
    return common::make_either(true, output.str());
  return common::make_either(state_ == State::CS0, output.str());
}

common::Either<bool, base::string16> EucJpDecoder::Private::Error() {
  return common::make_either(false, base::string16());
}

//////////////////////////////////////////////////////////////////////
//
// EucJpDecoder
//
EucJpDecoder::EucJpDecoder() : private_(new Private()) {}

EucJpDecoder::~EucJpDecoder() {}

// encoding::Decoder
const base::string16& EucJpDecoder::name() const {
  CR_DEFINE_STATIC_LOCAL(base::string16, name_, (static_name()));
  return name_;
}

common::Either<bool, base::string16> EucJpDecoder::Decode(const uint8_t* bytes,
                                                          size_t num_bytes,
                                                          bool is_stream) {
  return private_->Decode(bytes, num_bytes, is_stream);
}

}  // namespace encodings
