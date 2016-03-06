// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_ENCODINGS_EUC_JP_ENCODER_H_
#define EVITA_TEXT_ENCODINGS_EUC_JP_ENCODER_H_

#include <memory>
#include <string>
#include <vector>

#include "evita/text/encodings/encoder.h"

namespace encodings {

// EUC-JP Charset encoder.
class EucJpEncoder final : public Encoder {
 public:
  EucJpEncoder();
  ~EucJpEncoder() final;

  static const base::char16* static_name() { return L"euc-jp"; }

 private:
  class Private;

  // encoding::Encoder
  const base::string16& name() const final;
  base::Either<base::char16, std::vector<uint8_t>> Encode(
      const base::string16& string,
      bool is_stream) final;

  std::unique_ptr<Private> private_;

  DISALLOW_COPY_AND_ASSIGN(EucJpEncoder);
};

}  // namespace encodings

#endif  // EVITA_TEXT_ENCODINGS_EUC_JP_ENCODER_H_
