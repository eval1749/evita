// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_encoding_text_encoder_h)
#define INCLUDE_evita_dom_encoding_text_encoder_h

#include "evita/v8_glue/scriptable.h"

#include <memory.h>

#include "base/strings/string16.h"
#include "evita/v8_glue/optional.h"

namespace gin {
class ArrayBufferView;
}

namespace dom {

struct TextEncoderOptions {
  bool fatal;
};

struct TextEncodeOptions {
  bool stream;
};

class TextEncoder : public v8_glue::Scriptable<TextEncoder> {
  DECLARE_SCRIPTABLE_OBJECT(TextEncoder);

  private: class Encoder;

  private: std::unique_ptr<Encoder> encoder_;

  public: TextEncoder(const base::string16& label);
  public: virtual ~TextEncoder();

  public: const base::string16& encoding() const;

  public: v8::Handle<v8::Uint8Array> Encode(
      v8_glue::Optional<base::string16> opt_input,
      v8_glue::Optional<TextEncodeOptions> options);

  DISALLOW_COPY_AND_ASSIGN(TextEncoder);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_encoding_text_encoder_h)
