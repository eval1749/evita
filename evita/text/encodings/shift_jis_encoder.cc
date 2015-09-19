// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <windows.h>

#include "evita/text/encodings/shift_jis_encoder.h"

#include "base/logging.h"
#include "common/win/win32_verify.h"

namespace encodings {

//////////////////////////////////////////////////////////////////////
//
// ShiftJisEncoder::Private
//
class ShiftJisEncoder::Private final {
 public:
  Private();
  ~Private();

  common::Either<base::char16, std::vector<uint8_t>> Encode(
      const base::string16& string,
      bool is_stream);

 private:
  DISALLOW_COPY_AND_ASSIGN(Private);
};

ShiftJisEncoder::Private::Private() {}

ShiftJisEncoder::Private::~Private() {}

static size_t EncodeShiftJis(const base::string16 string,
                             uint8_t* bytes,
                             size_t max_bytes) {
  auto const kShiftJisCodePage = 932;
  BOOL used_default_char = FALSE;
  auto const num_bytes = static_cast<size_t>(::WideCharToMultiByte(
      kShiftJisCodePage, 0, string.data(), static_cast<int>(string.size()),
      reinterpret_cast<char*>(bytes), static_cast<int>(max_bytes), nullptr,
      &used_default_char));
  if (used_default_char) {
    ::SetLastError(ERROR_NO_UNICODE_TRANSLATION);
    return 0;
  }
  return num_bytes;
}

common::Either<base::char16, std::vector<uint8_t>>
ShiftJisEncoder::Private::Encode(const base::string16& string, bool) {
  if (string.empty()) {
    return common::make_either(static_cast<base::char16>(0),
                               std::vector<uint8_t>());
  }
  auto const num_bytes = EncodeShiftJis(string, nullptr, 0u);
  if (!num_bytes) {
    auto const last_error = ::GetLastError();
    if (last_error != ERROR_NO_UNICODE_TRANSLATION) {
      return common::make_either(static_cast<base::char16>(1),
                                 std::vector<uint8_t>());
    }
    for (auto const code_point : string) {
      base::string16 candidate(1u, code_point);
      if (!EncodeShiftJis(candidate, nullptr, 0u))
        return common::make_either(code_point, std::vector<uint8_t>());
    }
    NOTREACHED();
    return common::make_either(static_cast<base::char16>(1),
                               std::vector<uint8_t>());
  }
  std::vector<uint8_t> output(num_bytes);
  WIN32_VERIFY(EncodeShiftJis(string, &output[0], output.size()));
  return common::make_either(static_cast<base::char16>(0), output);
}

//////////////////////////////////////////////////////////////////////
//
// ShiftJisEncoder
//
ShiftJisEncoder::ShiftJisEncoder() : private_(new Private()) {}

ShiftJisEncoder::~ShiftJisEncoder() {}

// encoding::Encoder
const base::string16& ShiftJisEncoder::name() const {
  CR_DEFINE_STATIC_LOCAL(base::string16, name_, (static_name()));
  return name_;
}

common::Either<base::char16, std::vector<uint8_t>> ShiftJisEncoder::Encode(
    const base::string16& string,
    bool is_stream) {
  return private_->Encode(string, is_stream);
}

}  // namespace encodings
