// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_encodings_decoder_h)
#define INCLUDE_evita_encodings_decoder_h

#include <stdint.h>

#include "base/basictypes.h"
#include "base/strings/string16.h"
#include "common/either.h"

namespace encodings {

class Decoder {
  protected: Decoder();
  public: virtual ~Decoder();

  public: virtual const base::string16& name() const = 0;

  public: virtual common::Either<bool, base::string16> Decode(
      const uint8_t* bytes, size_t num_bytes, bool is_stream) = 0;

  DISALLOW_COPY_AND_ASSIGN(Decoder);
};  // Decoder

}  // namespace encodings

#endif //!defined(INCLUDE_evita_encodings_decoder_h)
