// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_ENCODINGS_DECODER_H_
#define EVITA_TEXT_ENCODINGS_DECODER_H_

#include <cstdint>

#include "base/basictypes.h"
#include "base/strings/string16.h"
#include "common/either.h"

namespace encodings {

class Decoder {
 public:
  virtual ~Decoder();

  virtual const base::string16& name() const = 0;

  virtual common::Either<bool, base::string16> Decode(const uint8_t* bytes,
                                                      size_t num_bytes,
                                                      bool is_stream) = 0;

 protected:
  Decoder();

 private:
  DISALLOW_COPY_AND_ASSIGN(Decoder);
};

}  // namespace encodings

#endif  // EVITA_TEXT_ENCODINGS_DECODER_H_
