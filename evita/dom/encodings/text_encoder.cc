// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "evita/dom/encodings/text_encoder.h"

#include "base/strings/stringprintf.h"
#include "evita/bindings/v8_glue_TextEncodeOptions.h"
#include "evita/dom/bindings/exception_state.h"
#include "evita/text/encodings/encoder.h"
#include "evita/text/encodings/encodings.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// TextEncoder
//
TextEncoder::TextEncoder(encodings::Encoder* encoder) : encoder_(encoder) {}

TextEncoder::~TextEncoder() {}

const base::string16& TextEncoder::encoding() const {
  return encoder_->name();
}

static std::vector<uint8_t> DoEncode(encodings::Encoder* encoder,
                                     const base::string16& string,
                                     bool is_stream,
                                     ExceptionState* exception_state) {
  auto const result = encoder->Encode(string, is_stream);
  if (result.left) {
    exception_state->ThrowError(
        base::StringPrintf("EncodingError: codePoint=0x%X", result.left));
    return std::vector<uint8_t>();
  }
  return result.right;
}

std::vector<uint8_t> TextEncoder::Encode(const base::string16& input,
                                         const TextEncodeOptions& options,
                                         ExceptionState* exception_state) {
  return DoEncode(encoder_.get(), input, options.stream(), exception_state);
}

std::vector<uint8_t> TextEncoder::Encode(const base::string16& input,
                                         ExceptionState* exception_state) {
  return DoEncode(encoder_.get(), input, false, exception_state);
}

std::vector<uint8_t> TextEncoder::Encode(ExceptionState* exception_state) {
  return DoEncode(encoder_.get(), base::string16(), false, exception_state);
}

TextEncoder* TextEncoder::NewTextEncoder(const base::string16& label,
                                         ExceptionState* exception_state) {
  auto const encoder = encodings::Encodings::instance()->GetEncoder(label);
  if (!encoder) {
    exception_state->ThrowError(
        base::StringPrintf("No such encoding '%ls'", label.c_str()));
    return nullptr;
  }
  return new TextEncoder(encoder);
}

TextEncoder* TextEncoder::NewTextEncoder(ExceptionState* exception_state) {
  return NewTextEncoder(L"utf-8", exception_state);
}

}  // namespace dom
