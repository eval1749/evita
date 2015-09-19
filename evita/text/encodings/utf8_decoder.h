// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_ENCODINGS_UTF8_DECODER_H_
#define EVITA_TEXT_ENCODINGS_UTF8_DECODER_H_

#include <memory>
#include <vector>

#include "evita/text/encodings/decoder.h"

namespace encodings {

// UTF-8 Charset decoder.
class Utf8Decoder final : public Decoder {
 public:
  Utf8Decoder();
  ~Utf8Decoder() final;

  static const base::char16* static_name() { return L"utf-8"; }

 private:
  class Private;

  // encoding::Decoder
  const base::string16& name() const final;
  common::Either<bool, base::string16> Decode(const uint8_t* bytes,
                                              size_t num_bytes,
                                              bool is_stream) final;

  std::unique_ptr<Private> private_;

  DISALLOW_COPY_AND_ASSIGN(Utf8Decoder);
};

}  // namespace encodings

#endif  // EVITA_TEXT_ENCODINGS_UTF8_DECODER_H_
