// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/encodings/text_encoder.h"

#include "base/strings/stringprintf.h"
#include "evita/bindings/TextEncodeOptions.h"
#include "evita/dom/script_host.h"
#include "evita/text/encodings/encoder.h"
#include "evita/text/encodings/encodings.h"
#include "evita/v8_glue/array_buffer_view.h"
#include "evita/v8_glue/converter.h"
#include "v8_strings.h"

namespace dom {

namespace {
//////////////////////////////////////////////////////////////////////
//
// TextEncoderClass
//
class TextEncoderClass : public v8_glue::WrapperInfo {
  private: typedef v8_glue::WrapperInfo BaseClass;

  public: TextEncoderClass(const char* name)
      : BaseClass(name) {
  }
  public: ~TextEncoderClass() = default;

  private: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate,
        &TextEncoder::NewTextEncoder);
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("encoding", &TextEncoder::encoding)
        .SetMethod("encode", &TextEncoder::Encode);
  }

  DISALLOW_COPY_AND_ASSIGN(TextEncoderClass);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TextEncoder
//
DEFINE_SCRIPTABLE_OBJECT(TextEncoder, TextEncoderClass);

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
    v8_glue::Optional<base::string16> opt_input,
    v8_glue::Optional<TextEncodeOptions> opt_options) {
  if (!opt_input.is_supplied)
    return DoEncode(encoder_.get(), base::string16(), false);
  auto const is_stream = opt_options.is_supplied && opt_options.value.stream();
  return DoEncode(encoder_.get(), opt_input.value, is_stream);
}

TextEncoder* TextEncoder::NewTextEncoder(
      v8_glue::Optional<base::string16> opt_label) {
  if (!opt_label.is_supplied) {
    return new TextEncoder(encodings::Encodings::instance()->
                                GetEncoder(L"utf-8"));
  }
  auto const encoder = encodings::Encodings::instance()->GetEncoder(
      opt_label.value);
  if (!encoder) {
    ScriptHost::instance()->ThrowError(base::StringPrintf(
        "No such encoding '%ls'", opt_label.value.c_str()));
    return nullptr;
  }
  return new TextEncoder(encoder);
}

}  // namespace dom
