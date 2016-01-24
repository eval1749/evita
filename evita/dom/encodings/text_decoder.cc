// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/encodings/text_decoder.h"

#include "base/strings/stringprintf.h"
#include "evita/bindings/v8_glue_TextDecodeOptions.h"
#include "evita/bindings/v8_glue_TextDecoderOptions.h"
#include "evita/dom/bindings/exception_state.h"
#include "evita/text/encodings/decoder.h"
#include "evita/text/encodings/encodings.h"
#include "evita/v8_glue/array_buffer_view.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// TextDecoder
//
TextDecoder::TextDecoder(encodings::Decoder* decoder,
                         const TextDecoderOptions& options)
    : decoder_(decoder), fatal_(options.fatal()) {}

TextDecoder::~TextDecoder() {}

const base::string16& TextDecoder::encoding() const {
  return decoder_->name();
}

base::string16 TextDecoder::Decode(const gin::ArrayBufferView& input,
                                   const TextDecodeOptions& options,
                                   ExceptionState* exception_state) {
  auto const result =
      decoder_->Decode(reinterpret_cast<const uint8_t*>(input.bytes()),
                       input.num_bytes(), options.stream());
  if (!result.left && fatal_)
    exception_state->ThrowError("Failed to decode data");
  return result.right;
}

base::string16 TextDecoder::Decode(const gin::ArrayBufferView& input,
                                   ExceptionState* exception_state) {
  auto const result =
      decoder_->Decode(reinterpret_cast<const uint8_t*>(input.bytes()),
                       input.num_bytes(), false);
  if (!result.left && fatal_)
    exception_state->ThrowError("Failed to decode data");
  return result.right;
}

base::string16 TextDecoder::Decode(ExceptionState* exception_state) {
  auto const result = decoder_->Decode(nullptr, 0, false);
  if (!result.left && fatal_)
    exception_state->ThrowError("Failed to decode data");
  return result.right;
}

// static
TextDecoder* TextDecoder::NewTextDecoder(const base::string16& label,
                                         const TextDecoderOptions& options,
                                         ExceptionState* exception_state) {
  auto const decoder = encodings::Encodings::instance()->GetDecoder(label);
  if (!decoder) {
    exception_state->ThrowError(
        base::StringPrintf("No such encoding '%ls'", label.c_str()));
    return nullptr;
  }
  return new TextDecoder(decoder, options);
}

TextDecoder* TextDecoder::NewTextDecoder(const base::string16& label,
                                         ExceptionState* exception_state) {
  return NewTextDecoder(label, TextDecoderOptions(), exception_state);
}

TextDecoder* TextDecoder::NewTextDecoder(ExceptionState* exception_state) {
  return NewTextDecoder(L"utf-8", exception_state);
}

}  // namespace dom
