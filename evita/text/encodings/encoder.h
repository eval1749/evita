// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_encodings_encoder_h)
#define INCLUDE_evita_encodings_encoder_h

#include <stdint.h>
#include <vector>

#include "base/basictypes.h"
#include "base/strings/string16.h"
#include "common/either.h"

namespace encodings {

class Encoder {
  protected: Encoder();
  public: virtual ~Encoder();

  public: virtual const base::string16& name() const = 0;

  public: virtual common::Either<base::char16, std::vector<uint8_t>> Encode(
      const base::string16& string, bool is_stream) = 0;

  DISALLOW_COPY_AND_ASSIGN(Encoder);
};  // Encoder

}  // namespace encodings

#endif //!defined(INCLUDE_evita_encodings_encoder_h)
