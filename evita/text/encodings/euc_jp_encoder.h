// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_encodings_euc_jp_encoder_h)
#define INCLUDE_evita_encodings_euc_jp_encoder_h

#include "evita/text/encodings/encoder.h"

#include <memory>

namespace encodings {

// UTF-8 Charset encoder.
class EucJpEncoder : public Encoder {
  private: class Private;

  private: std::unique_ptr<Private> private_;

  public: EucJpEncoder();
  public: virtual ~EucJpEncoder();

  public: static const base::char16* static_name() { return L"euc-jp"; }

  // encoding::Encoder
  private: virtual const base::string16& name() const override;
  private: virtual common::Either<base::char16, std::vector<uint8_t>>
      Encode(const base::string16& string, bool is_stream) override;

  DISALLOW_COPY_AND_ASSIGN(EucJpEncoder);
};  // EucJpEncoder

}  // namespace encodings

#endif //!defined(INCLUDE_evita_encodings_euc_jp_encoder_h)
