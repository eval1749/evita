// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>
#include <string>

#include "evita/text/encodings/utf8_decoder.h"

#include "base/logging.h"

namespace encodings {

//////////////////////////////////////////////////////////////////////
//
// Utf8Decoder::Private
//
class Utf8Decoder::Private final {
 public:
  Private();
  ~Private();

  // |string| contains characters decoded so far. Caller may use it what's
  // wrong in input data.
  common::Either<bool, base::string16> BadInput(const base::string16& string);

  common::Either<bool, base::string16> Decode(const uint8_t* bytes,
                                              size_t num_bytes,
                                              bool is_stream);

 private:
  enum class State {
    BadInput,
    FirstByte,
    NeedByte,
  };

  int char32_;
  int num_bytes_needed_;
  State state_;

  DISALLOW_COPY_AND_ASSIGN(Private);
};

Utf8Decoder::Private::Private()
    : char32_(0), num_bytes_needed_(0), state_(State::FirstByte) {}

Utf8Decoder::Private::~Private() {}

common::Either<bool, base::string16> Utf8Decoder::Private::BadInput(
    const base::string16& string) {
  state_ = State::BadInput;
  return common::make_either(false, string);
}

// 1 U+0000   U+007F   0xxxxxxx
// 2 U+0080   U+07FF   110xxxxx 10xxxxxx
// 3 U+0800   U+FFFF   1110xxxx 10xxxxxx 10xxxxxx
// 4 U+10000  U+1FFFFF 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
// Note: We don't support 5 byte and 6 byte UTF-8 sequence yet.
common::Either<bool, base::string16> Utf8Decoder::Private::Decode(
    const uint8_t* bytes,
    size_t num_bytes,
    bool is_stream) {
  if (!is_stream) {
    // Reset decode stream when we start decoding.
    state_ = State::FirstByte;
    num_bytes_needed_ = 0;
  }

  std::basic_stringbuf<base::char16> output;
  auto const bytes_end = bytes + num_bytes;
  for (auto runner = bytes; runner < bytes_end; ++runner) {
    auto const byte = *runner;
    switch (state_) {
      case State::BadInput:
        return BadInput(base::string16());
      case State::FirstByte:
        if (byte <= 0x7F) {
          output.sputc(static_cast<base::char16>(byte));
          break;
        }
        if (byte >= 0xC0 && byte <= 0xDF) {
          char32_ = byte & 0x1F;
          num_bytes_needed_ = 1;
          state_ = State::NeedByte;
          break;
        }
        if (byte >= 0xE0 && byte <= 0xEF) {
          char32_ = byte & 0x0F;
          num_bytes_needed_ = 2;
          state_ = State::NeedByte;
          break;
        }
        if (byte >= 0xF0 && byte <= 0xF4) {
          char32_ = byte & 7;
          num_bytes_needed_ = 3;
          state_ = State::NeedByte;
          break;
        }
        return BadInput(output.str());
      case State::NeedByte:
        if (byte < 0x80 || byte > 0xBF)
          return BadInput(output.str());
        char32_ <<= 6;
        char32_ |= byte & 0x3F;
        --num_bytes_needed_;
        if (num_bytes_needed_)
          break;
        if (char32_ <= 0xFFFF) {
          output.sputc(static_cast<base::char16>(char32_));
        } else if (char32_ <= 0x10FFFF) {
          char32_ -= 0x10000;
          output.sputc(
              static_cast<base::char16>(0xD800 | ((char32_ >> 10) & 0x3FF)));
          output.sputc(static_cast<base::char16>(0xDC00 | (char32_ & 0x3FF)));
        } else {
          return BadInput(output.str());
        }
        state_ = State::FirstByte;
        break;
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
Utf8Decoder::Utf8Decoder() : private_(new Private()) {}

Utf8Decoder::~Utf8Decoder() {}

// encoding::Decoder
const base::string16& Utf8Decoder::name() const {
  CR_DEFINE_STATIC_LOCAL(base::string16, name_, (static_name()));
  return name_;
}

common::Either<bool, base::string16> Utf8Decoder::Decode(const uint8_t* bytes,
                                                         size_t num_bytes,
                                                         bool is_stream) {
  return private_->Decode(bytes, num_bytes, is_stream);
}

}  // namespace encodings
