// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_ENCODINGS_TEXT_ENCODER_H_
#define EVITA_DOM_ENCODINGS_TEXT_ENCODER_H_

#include <memory>
#include <vector>

#include "evita/v8_glue/scriptable.h"

#include "base/strings/string16.h"

namespace encodings {
class Encoder;
}

namespace dom {

namespace bindings {
class TextEncoderClass;
}

class ExceptionState;
class TextEncodeOptions;

class TextEncoder final : public v8_glue::Scriptable<TextEncoder> {
  DECLARE_SCRIPTABLE_OBJECT(TextEncoder);

 public:
  ~TextEncoder() final;

 private:
  friend class bindings::TextEncoderClass;

  explicit TextEncoder(encodings::Encoder* encoder);

  // bindings
  const base::string16& encoding() const;

  std::vector<uint8_t> Encode(const base::string16& input,
                              const TextEncodeOptions& options,
                              ExceptionState* exception_state);
  std::vector<uint8_t> Encode(const base::string16& input,
                              ExceptionState* exception_state);
  std::vector<uint8_t> Encode(ExceptionState* exception_state);

  static TextEncoder* NewTextEncoder(const base::string16& label,
                                     ExceptionState* exception_state);
  static TextEncoder* NewTextEncoder(ExceptionState* exception_state);

  std::unique_ptr<encodings::Encoder> encoder_;

  DISALLOW_COPY_AND_ASSIGN(TextEncoder);
};

}  // namespace dom

#endif  // EVITA_DOM_ENCODINGS_TEXT_ENCODER_H_
