// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_encodings_encodings_h)
#define INCLUDE_evita_encodings_encodings_h

#include <memory>

#include "base/basictypes.h"
#include "base/strings/string16.h"
#include "common/memory/singleton.h"

namespace encodings {

class Decoder;
class Encoder;

class Encodings : public common::Singleton<Encodings> {
  DECLARE_SINGLETON_CLASS(Encodings);

  private: class Private;

  private: std::unique_ptr<Private> private_;

  private: Encodings();
  public: ~Encodings();

  public: Decoder* GetDecoder(const base::string16& name) const;
  public: Encoder* GetEncoder(const base::string16& name) const;

  DISALLOW_COPY_AND_ASSIGN(Encodings);
};  // Encodings

}  // namespace encodings

#endif //!defined(INCLUDE_evita_encodings_encodings_h)
