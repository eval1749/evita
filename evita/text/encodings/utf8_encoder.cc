// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>

#include "evita/text/encodings/utf8_encoder.h"

#include "base/logging.h"

namespace encodings {

//////////////////////////////////////////////////////////////////////
//
// Utf8Encoder::Private
//
class Utf8Encoder::Private final {
 public:
  Private();
  ~Private();

  common::Either<base::char16, std::vector<uint8_t>> Encode(
      const base::string16& string,
      bool is_stream);

 private:
  DISALLOW_COPY_AND_ASSIGN(Private);
};

Utf8Encoder::Private::Private() {}

Utf8Encoder::Private::~Private() {}

// U+0000..U+007F 0xxxxxxx
// U+0080..U+07FF 110xxxxx 10xxxxxx
// U+0800..U+FFFF 1110xxxx 10xxxxxx 10xxxxxx
common::Either<base::char16, std::vector<uint8_t>> Utf8Encoder::Private::Encode(
    const base::string16& string,
    bool) {
  std::vector<uint8_t> output(string.size());
  output.resize(0);
  for (auto const code_point : string) {
    if (code_point >= 0xD800 && code_point <= 0xDFFF) {
      output.resize(output.size());
      return common::make_either(code_point, output);
    }
    if (code_point <= 0x7F) {
      output.push_back(static_cast<uint8_t>(code_point));
      continue;
    }
    if (code_point <= 0x7FF) {
      output.push_back(static_cast<uint8_t>(0xC0 | ((code_point >> 6) & 0x1F)));
    } else {
      output.push_back(
          static_cast<uint8_t>(0xE0 | ((code_point >> 12) & 0x0F)));
      output.push_back(static_cast<uint8_t>(0x80 | ((code_point >> 6) & 0x3F)));
    }
    output.push_back(static_cast<uint8_t>(0x80 | (code_point & 0x3F)));
  }
  output.resize(output.size());
  return common::make_either(static_cast<base::char16>(0), output);
}

//////////////////////////////////////////////////////////////////////
//
// Utf8Encoder
//
Utf8Encoder::Utf8Encoder() : private_(new Private()) {}

Utf8Encoder::~Utf8Encoder() {}

// encoding::Encoder
const base::string16& Utf8Encoder::name() const {
  CR_DEFINE_STATIC_LOCAL(base::string16, name_, (static_name()));
  return name_;
}

common::Either<base::char16, std::vector<uint8_t>> Utf8Encoder::Encode(
    const base::string16& string,
    bool is_stream) {
  return private_->Encode(string, is_stream);
}

}  // namespace encodings
