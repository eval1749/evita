// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_encoding_text_decoder_h)
#define INCLUDE_evita_dom_encoding_text_decoder_h

#include "evita/v8_glue/scriptable.h"

#include <memory.h>

#include "base/strings/string16.h"
#include "evita/v8_glue/optional.h"

namespace encodings {
class Decoder;
}

namespace gin {
class ArrayBufferView;
}

namespace dom {

struct TextDecoderOptions {
  bool fatal;
};

struct TextDecodeOptions {
  bool stream;
};

class TextDecoder : public v8_glue::Scriptable<TextDecoder> {
  DECLARE_SCRIPTABLE_OBJECT(TextDecoder);

  private: std::unique_ptr<encodings::Decoder> decoder_;
  private: bool fatal_;

  public: TextDecoder(encodings::Decoder* decoder,
                      const TextDecoderOptions& options);
  public: virtual ~TextDecoder();

  public: const base::string16& encoding() const;

  public: base::string16 Decode(
      v8_glue::Optional<gin::ArrayBufferView*> opt_input,
      v8_glue::Optional<TextDecodeOptions> options);

  DISALLOW_COPY_AND_ASSIGN(TextDecoder);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_encoding_text_decoder_h)
