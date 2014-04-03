// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/encodings/euc_jp_decoder.h"

#include <sstream>

#include "base/logging.h"

namespace encodings {

//////////////////////////////////////////////////////////////////////
//
// EucJpDecoder::Private
//
class EucJpDecoder::Private {
  private: enum class State {
    FirstByte,
    SecondByteOfTwoByteEncoding,
    SecondByteOfThreeByteEncoding,
    ThirdByteOfThreeByteEncoding,
  };

  private: int char16_;
  private: State state_;

  public: Private();
  public: ~Private();

  public: common::Either<bool, base::string16> Decode(
      const uint8_t* bytes, size_t num_bytes, bool is_stream);

  DISALLOW_COPY_AND_ASSIGN(Private);
};  // EucJpDecoder

EucJpDecoder::Private::Private() : state_(State::FirstByte) {
}

EucJpDecoder::Private::~Private() {
}

common::Either<bool, base::string16> EucJpDecoder::Private::Decode(
      const uint8_t* bytes, size_t num_bytes, bool is_stream) {
  std::basic_stringbuf<base::char16> output;
  auto const bytes_end = bytes + num_bytes;
  for (auto runner = bytes; runner < bytes_end; ++runner) {
    auto const byte = *runner;
    switch (state_) {
      case State::FirstByte:
        if (byte <= 0x7E) {
          output.sputc(static_cast<base::char16>(byte));
          break;
        }
        if (byte >= 0xC0 && byte <= 0xDF) {
          char16_ = (byte & 0x1F) << 6;
          state_ = State::SecondByteOfTwoByteEncoding;
          break;
        }
        if (byte >= 0xE0 && byte <= 0xEF) {
          char16_ = (byte & 0x0F) << 12;
          state_ = State::SecondByteOfThreeByteEncoding;
          break;
        }
        return common::make_either(is_stream, output.str());
      case State::SecondByteOfTwoByteEncoding:
        if (byte >= 0x80 && byte <= 0xBF) {
          char16_ |= byte & 0x3F;
          output.sputc(static_cast<base::char16>(char16_));
          state_ = State::FirstByte;
          break;
        }
        return common::make_either(is_stream, output.str());
      case State::SecondByteOfThreeByteEncoding:
        if (byte >= 0x80 && byte <= 0xBF) {
          char16_ |= (byte & 0x3F) << 6;
          state_ = State::ThirdByteOfThreeByteEncoding;
          break;
        }
        return common::make_either(is_stream, output.str());
      case State::ThirdByteOfThreeByteEncoding:
        if (byte >= 0x80 && byte <= 0xBF) {
          char16_ = byte & 0x3F;
          output.sputc(static_cast<base::char16>(char16_));
          state_ = State::FirstByte;
          break;
        }
        return common::make_either(is_stream, output.str());
      default:
        NOTREACHED();
        break;
    }
  }
  if (is_stream)
    return common::make_either(true, output.str());
  auto const error = state_ == State::FirstByte;
  state_ = State::FirstByte;
  return common::make_either(error, output.str());
}

//////////////////////////////////////////////////////////////////////
//
// EucJpDecoder
//
EucJpDecoder::EucJpDecoder() : private_(new Private()) {
}

EucJpDecoder::~EucJpDecoder() {
}

// encoding::Decoder
const base::string16& EucJpDecoder::name() const {
  CR_DEFINE_STATIC_LOCAL(base::string16, name_, (static_name()));
  return name_;
}

common::Either<bool, base::string16> EucJpDecoder::Decode(
      const uint8_t* bytes, size_t num_bytes, bool is_stream) {
  return private_->Decode(bytes, num_bytes, is_stream);
}

}  // namespace encodings
