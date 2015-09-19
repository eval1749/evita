// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_ENCODINGS_ENCODER_H_
#define EVITA_TEXT_ENCODINGS_ENCODER_H_

#include <cstdint>
#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/strings/string16.h"
#include "common/either.h"

namespace encodings {

class Encoder {
 public:
  virtual ~Encoder();

  virtual const base::string16& name() const = 0;

  virtual common::Either<base::char16, std::vector<uint8_t>> Encode(
      const base::string16& string,
      bool is_stream) = 0;

 protected:
  Encoder();

 private:
  DISALLOW_COPY_AND_ASSIGN(Encoder);
};

}  // namespace encodings

#endif  // EVITA_TEXT_ENCODINGS_ENCODER_H_
