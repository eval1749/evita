// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_encodings_shift_jp_decoder_h)
#define INCLUDE_evita_encodings_shift_jp_decoder_h

#include "evita/text/encodings/decoder.h"

#include <memory>

namespace encodings {

// Shift_JIS encoding decoder.
class ShiftJisDecoder : public Decoder {
  private: class Private;

  private: std::unique_ptr<Private> private_;

  public: ShiftJisDecoder();
  public: virtual ~ShiftJisDecoder();

  public: static const base::char16* static_name() { return L"shift_jis"; }

  // encoding::Decoder
  private: virtual const base::string16& name() const override;
  private: virtual common::Either<bool, base::string16> Decode(
      const uint8_t* bytes, size_t num_bytes, bool is_stream) override;

  DISALLOW_COPY_AND_ASSIGN(ShiftJisDecoder);
};  // ShiftJisDecoder

}  // namespace encodings

#endif //!defined(INCLUDE_evita_encodings_shift_jp_decoder_h)
