// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_ENCODINGS_TEXT_DECODER_H_
#define EVITA_DOM_ENCODINGS_TEXT_DECODER_H_

#include <memory>

#include "evita/ginx/scriptable.h"

#include "base/strings/string16.h"

namespace encodings {
class Decoder;
}

namespace gin {
class ArrayBufferView;
}

namespace dom {

class ExceptionState;
class TextDecodeOptions;
class TextDecoderOptions;

namespace bindings {
class TextDecoderClass;
}

class TextDecoder final : public ginx::Scriptable<TextDecoder> {
  DECLARE_SCRIPTABLE_OBJECT(TextDecoder);

 public:
  ~TextDecoder() final;

  static TextDecoder* NewTextDecoder(ExceptionState* exception_state);
  static TextDecoder* NewTextDecoder(const base::string16& encoding,
                                     ExceptionState* exception_state);
  static TextDecoder* NewTextDecoder(const base::string16& encoding,
                                     const TextDecoderOptions& options,
                                     ExceptionState* exception_state);

 private:
  friend class bindings::TextDecoderClass;

  TextDecoder(encodings::Decoder* decoder, const TextDecoderOptions& options);

  const base::string16& encoding() const;

  base::string16 Decode(ExceptionState* exception_state);
  base::string16 Decode(const gin::ArrayBufferView& input,
                        ExceptionState* exception_state);
  base::string16 Decode(const gin::ArrayBufferView& input,
                        const TextDecodeOptions& options,
                        ExceptionState* exception_state);

  std::unique_ptr<encodings::Decoder> decoder_;
  bool fatal_;

  DISALLOW_COPY_AND_ASSIGN(TextDecoder);
};

}  // namespace dom

#endif  // EVITA_DOM_ENCODINGS_TEXT_DECODER_H_
