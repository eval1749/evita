// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_ENCODINGS_ENCODINGS_H_
#define EVITA_TEXT_ENCODINGS_ENCODINGS_H_

#include <memory>

#include "base/basictypes.h"
#include "base/strings/string16.h"
#include "common/memory/singleton.h"

namespace encodings {

class Decoder;
class Encoder;

class Encodings final : public common::Singleton<Encodings> {
  DECLARE_SINGLETON_CLASS(Encodings);

 public:
  Decoder* GetDecoder(const base::string16& name) const;
  Encoder* GetEncoder(const base::string16& name) const;

 private:
  ~Encodings() final;

 private:
  class Private;

  Encodings();

  std::unique_ptr<Private> private_;

  DISALLOW_COPY_AND_ASSIGN(Encodings);
};  // Encodings

}  // namespace encodings

#endif  // EVITA_TEXT_ENCODINGS_ENCODINGS_H_
