// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_encoding_text_encoder_h)
#define INCLUDE_evita_dom_encoding_text_encoder_h

#include "evita/v8_glue/scriptable.h"

#include <memory.h>

#include "base/strings/string16.h"

namespace encodings {
class Encoder;
}

namespace dom {

namespace bindings {
class TextEncoderClass;
}

class TextEncodeOptions;

class TextEncoder : public v8_glue::Scriptable<TextEncoder> {
  DECLARE_SCRIPTABLE_OBJECT(TextEncoder);
  friend class bindings::TextEncoderClass;

  private: std::unique_ptr<encodings::Encoder> encoder_;

  private: TextEncoder(encodings::Encoder* encoder);
  public: virtual ~TextEncoder();

  private: const base::string16& encoding() const;

  private: std::vector<uint8_t> Encode(
      const base::string16& input,
      const TextEncodeOptions& options);
  private: std::vector<uint8_t> Encode(const base::string16& input);
  private: std::vector<uint8_t> Encode();

  private: static TextEncoder* NewTextEncoder(const base::string16& label);
  private: static TextEncoder* NewTextEncoder();

  DISALLOW_COPY_AND_ASSIGN(TextEncoder);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_encoding_text_encoder_h)
