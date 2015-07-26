// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/encodings/text_encoder.h"

#include "base/strings/stringprintf.h"
#include "evita/bindings/v8_glue_TextEncodeOptions.h"
#include "evita/dom/script_host.h"
#include "evita/text/encodings/encoder.h"
#include "evita/text/encodings/encodings.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// TextEncoder
//
TextEncoder::TextEncoder(encodings::Encoder* encoder) : encoder_(encoder) {
}

TextEncoder::~TextEncoder() {
}

const base::string16& TextEncoder::encoding() const {
  return encoder_->name();
}

static std::vector<uint8_t> DoEncode(encodings::Encoder* encoder,
                                     const base::string16& string,
                                     bool is_stream) {
  auto const result = encoder->Encode(string, is_stream);
  if (result.left) {
    ScriptHost::instance()->ThrowError(base::StringPrintf(
        "EncodingError: codePoint=0x%X", result.left));
    return std::vector<uint8_t>();
  }
  return result.right;
}

std::vector<uint8_t> TextEncoder::Encode(
    const base::string16& input,
    const TextEncodeOptions& options) {
  return DoEncode(encoder_.get(), input, options.stream());
}

std::vector<uint8_t> TextEncoder::Encode(const base::string16& input) {
  return DoEncode(encoder_.get(), input, false);
}

std::vector<uint8_t> TextEncoder::Encode() {
  return DoEncode(encoder_.get(), base::string16(), false);
}

TextEncoder* TextEncoder::NewTextEncoder(const base::string16& label) {
  auto const encoder = encodings::Encodings::instance()->GetEncoder(label);
  if (!encoder) {
    ScriptHost::instance()->ThrowError(base::StringPrintf(
        "No such encoding '%ls'", label.c_str()));
    return nullptr;
  }
  return new TextEncoder(encoder);
}

TextEncoder* TextEncoder::NewTextEncoder() {
  return NewTextEncoder(L"utf-8");
}

}  // namespace dom
