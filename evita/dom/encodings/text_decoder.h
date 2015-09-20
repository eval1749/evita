// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_ENCODINGS_TEXT_DECODER_H_
#define EVITA_DOM_ENCODINGS_TEXT_DECODER_H_

#include <memory>

#include "evita/v8_glue/scriptable.h"

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

class TextDecoder final : public v8_glue::Scriptable<TextDecoder> {
  DECLARE_SCRIPTABLE_OBJECT(TextDecoder);

 public:
  ~TextDecoder() final;

  static TextDecoder* NewTextDecoder();
  static TextDecoder* NewTextDecoder(const base::string16& encoding);
  static TextDecoder* NewTextDecoder(const base::string16& encoding,
                                     const TextDecoderOptions& options);

 private:
  friend class bindings::TextDecoderClass;

  TextDecoder(encodings::Decoder* decoder, const TextDecoderOptions& options);

  const base::string16& encoding() const;

  base::string16 Decode();
  base::string16 Decode(const gin::ArrayBufferView& input);
  base::string16 Decode(const gin::ArrayBufferView& input,
                        const TextDecodeOptions& options);

  std::unique_ptr<encodings::Decoder> decoder_;
  bool fatal_;

  DISALLOW_COPY_AND_ASSIGN(TextDecoder);
};

}  // namespace dom

#endif  // EVITA_DOM_ENCODINGS_TEXT_DECODER_H_
