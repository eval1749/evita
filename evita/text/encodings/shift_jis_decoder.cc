// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/encodings/shift_jis_decoder.h"

#include <windows.h>
#include <sstream>

#include "base/logging.h"

namespace encodings {

//////////////////////////////////////////////////////////////////////
//
// ShiftJisDecoder::Private
//
class ShiftJisDecoder::Private final {
 public:
  Private();
  ~Private();

  common::Either<bool, base::string16> Decode(const uint8_t* bytes,
                                              size_t num_bytes,
                                              bool is_stream);

 private:
  uint8_t bytes_[2];

  DISALLOW_COPY_AND_ASSIGN(Private);
};

ShiftJisDecoder::Private::Private() {
  bytes_[0] = 0;
  bytes_[1] = 0;
}

ShiftJisDecoder::Private::~Private() {}

common::Either<bool, base::string16> ShiftJisDecoder::Private::Decode(
    const uint8_t* bytes,
    size_t num_bytes,
    bool is_stream) {
  if (!is_stream) {
    bytes_[0] = 0;
    bytes_[1] = 0;
  }
  std::basic_stringbuf<base::char16> output;
  auto const bytes_end = bytes + num_bytes;
  for (auto runner = bytes; runner < bytes_end; ++runner) {
    auto const byte = *runner;
    if (bytes_[0]) {
      bytes_[1] = byte;
      auto const kShiftJisCodePage = 932;
      base::char16 code_point;
      auto const num_chars = ::MultiByteToWideChar(
          kShiftJisCodePage, MB_ERR_INVALID_CHARS,
          reinterpret_cast<char*>(bytes_), 2, &code_point, 1);
      if (num_chars != 1)
        return common::make_either(false, output.str());
      output.sputc(code_point);
      bytes_[0] = 0;
    } else {
      if (byte <= 0x80) {
        output.sputc(static_cast<base::char16>(byte));
      } else if (byte >= 0xA1 && byte <= 0xDF) {
        output.sputc(static_cast<base::char16>(0xFF61 + byte - 0xA1));
      } else if ((byte >= 0x81 && byte <= 0x9F) ||
                 (byte >= 0xE0 && byte <= 0xFC)) {
        bytes_[0] = byte;
      } else {
        return common::make_either(false, output.str());
      }
    }
  }
  if (is_stream)
    return common::make_either(true, output.str());
  return common::make_either(!bytes_[0], output.str());
}

//////////////////////////////////////////////////////////////////////
//
// ShiftJisDecoder
//
ShiftJisDecoder::ShiftJisDecoder() : private_(new Private()) {}

ShiftJisDecoder::~ShiftJisDecoder() {}

// encoding::Decoder
const base::string16& ShiftJisDecoder::name() const {
  CR_DEFINE_STATIC_LOCAL(base::string16, name_, (static_name()));
  return name_;
}

common::Either<bool, base::string16> ShiftJisDecoder::Decode(
    const uint8_t* bytes,
    size_t num_bytes,
    bool is_stream) {
  return private_->Decode(bytes, num_bytes, is_stream);
}

}  // namespace encodings
