// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/encodings/utf8_decoder.h"

#include <sstream>

#include "base/logging.h"

namespace encodings {

//////////////////////////////////////////////////////////////////////
//
// Utf8Decoder::Private
//
class Utf8Decoder::Private {
  private: enum class State {
    BadInput,
    FirstByte,
    LastByte,
    SecondByteOfThreeByteEncoding,
  };

  private: int char16_;
  private: State state_;

  public: Private();
  public: ~Private();

  // |string| contains characters decoded so far. Caller may use it what's
  // wrong in input data.
  private: common::Either<bool, base::string16> BadInput(
      const base::string16& string);

  public: common::Either<bool, base::string16> Decode(
      const uint8_t* bytes, size_t num_bytes, bool is_stream);

  DISALLOW_COPY_AND_ASSIGN(Private);
};  // Utf8Decoder

Utf8Decoder::Private::Private() : char16_(0), state_(State::FirstByte) {
}

Utf8Decoder::Private::~Private() {
}

common::Either<bool, base::string16> Utf8Decoder::Private::BadInput(
    const base::string16& string) {
  state_ = State::BadInput;
  return common::make_either(false, string);
}

// U+0000..U+007F 0xxxxxxx
// U+0080..U+07FF 110xxxxx 10xxxxxx
// U+0800..U+FFFF 1110xxxx 10xxxxxx 10xxxxxx
common::Either<bool, base::string16> Utf8Decoder::Private::Decode(
      const uint8_t* bytes, size_t num_bytes, bool is_stream) {
  if (!is_stream) {
    // Reset decode stream when we start decoding.
    state_ = State::FirstByte;
  }

  std::basic_stringbuf<base::char16> output;
  auto const bytes_end = bytes + num_bytes;
  for (auto runner = bytes; runner < bytes_end; ++runner) {
    auto const byte = *runner;
    switch (state_) {
      case State::BadInput:
        return BadInput(base::string16());
      case State::FirstByte:
        if (byte <= 0x7E) {
          output.sputc(static_cast<base::char16>(byte));
          break;
        }
        if (byte >= 0xC0 && byte <= 0xDF) {
          char16_ = (byte & 0x1F) << 6;
          state_ = State::LastByte;
          break;
        }
        if (byte >= 0xE0 && byte <= 0xEF) {
          char16_ = (byte & 0x0F) << 12;
          state_ = State::SecondByteOfThreeByteEncoding;
          break;
        }
        return BadInput(output.str());
      case State::LastByte:
        if (byte >= 0x80 && byte <= 0xBF) {
          char16_ |= byte & 0x3F;
          output.sputc(static_cast<base::char16>(char16_));
          state_ = State::FirstByte;
          break;
        }
        return BadInput(output.str());
      case State::SecondByteOfThreeByteEncoding:
        if (byte >= 0x80 && byte <= 0xBF) {
          char16_ |= (byte & 0x3F) << 6;
          state_ = State::LastByte;
          break;
        }
        return BadInput(output.str());
      default:
        NOTREACHED();
        break;
    }
  }
  if (is_stream)
    return common::make_either(true, output.str());

  // We should not expect more bytes.
  auto const error = state_ == State::FirstByte;
  state_ = State::FirstByte;
  return common::make_either(error, output.str());
}

//////////////////////////////////////////////////////////////////////
//
// Utf8Decoder
//
Utf8Decoder::Utf8Decoder() : private_(new Private()) {
}

Utf8Decoder::~Utf8Decoder() {
}

// encoding::Decoder
const base::string16& Utf8Decoder::name() const {
  CR_DEFINE_STATIC_LOCAL(base::string16, name_, (static_name()));
  return name_;
}

common::Either<bool, base::string16> Utf8Decoder::Decode(
      const uint8_t* bytes, size_t num_bytes, bool is_stream) {
  return private_->Decode(bytes, num_bytes, is_stream);
}

}  // namespace encodings
