// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_encoding_text_decoder_h)
#define INCLUDE_evita_dom_encoding_text_decoder_h

#include "evita/v8_glue/scriptable.h"

#include <memory.h>

#include "base/strings/string16.h"

namespace encodings {
class Decoder;
}

namespace gin {
class ArrayBufferView;
}

namespace dom {

class TextDecodeOptions;
class TextDecoderOptions;

namespace bindings {
class TextDecoderClass;
}

class TextDecoder : public v8_glue::Scriptable<TextDecoder> {
  DECLARE_SCRIPTABLE_OBJECT(TextDecoder);
  friend class bindings::TextDecoderClass;

  private: std::unique_ptr<encodings::Decoder> decoder_;
  private: bool fatal_;

  private: TextDecoder(encodings::Decoder* decoder,
                      const TextDecoderOptions& options);
  public: virtual ~TextDecoder();

  private: const base::string16& encoding() const;

  private: base::string16 Decode();
  private: base::string16 Decode(const gin::ArrayBufferView& input);
  private: base::string16 Decode(const gin::ArrayBufferView& input,
                                 const TextDecodeOptions& options);

  public: static TextDecoder* NewTextDecoder();
  public: static TextDecoder* NewTextDecoder(const base::string16& encoding);
  public: static TextDecoder* NewTextDecoder(const base::string16& encoding,
                                             const TextDecoderOptions& options);

  DISALLOW_COPY_AND_ASSIGN(TextDecoder);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_encoding_text_decoder_h)
