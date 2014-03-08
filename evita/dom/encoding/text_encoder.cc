// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/encoding/text_encoder.h"

#include "evita/dom/script_controller.h"
#include "evita/v8_glue/array_buffer_view.h"
#include "evita/v8_glue/converter.h"
#include "v8_strings.h"

namespace gin {

template<>
struct Converter<dom::TextEncodeOptions> {
  static bool FromV8(v8::Isolate* isolate,
                     v8::Handle<v8::Value> value,
                     dom::TextEncodeOptions* out) {
    if (!value->IsObject())
      return false;
    auto const options = value->ToObject();
    auto const stream = options->Get(dom::v8Strings::stream.Get(isolate));
    if (!ConvertFromV8(isolate, stream, &out->stream))
      return false;
    return true;
  }
};

}  // namespace gin

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
        &TextEncoderClass::NewTextEncoder);
  }

  private: static TextEncoder* NewTextEncoder(
      v8_glue::Optional<base::string16> opt_label) {
    return new TextEncoder(opt_label.is_supplied ? opt_label.value :
                                                   L"utf-8");
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
// TextEncoder::Encoder
//
class TextEncoder::Encoder {
  private: base::string16 encoding_;

  public: Encoder(const base::string16&);
  public: ~Encoder();

  public: const string16& encoding() const { return encoding_; }

  public: v8::Handle<v8::Uint8Array> Encode(v8::Isolate* isolate,
      const base::string16& input, bool is_stream);
};

TextEncoder::Encoder::Encoder(const base::string16&)
    : encoding_(L"utf-8") {
}

TextEncoder::Encoder::~Encoder() {
}

v8::Handle<v8::Uint8Array> TextEncoder::Encoder::Encode(v8::Isolate* isolate,
    const base::string16& input, bool) {
  auto const output_size = input.length();
  v8::EscapableHandleScope handle_scope(isolate);
  auto const buffer = v8::ArrayBuffer::New(isolate, output_size);
  auto const uint8_view = v8::Uint8Array::New(buffer, 0, output_size);
  gin::ArrayBufferView buffer_view(isolate, uint8_view);
  auto runner = reinterpret_cast<uint8_t*>(buffer_view.bytes());
  for (auto char_code : input) {
    *runner = static_cast<uint8_t>(char_code);
    ++runner;
  }
  return handle_scope.Escape(uint8_view);
}

//////////////////////////////////////////////////////////////////////
//
// TextEncoder
//
DEFINE_SCRIPTABLE_OBJECT(TextEncoder, TextEncoderClass);

TextEncoder::TextEncoder(const base::string16& label)
    : encoder_(new Encoder(label)) {
}

TextEncoder::~TextEncoder() {
}

const base::string16& TextEncoder::encoding() const {
  return encoder_->encoding();
}

v8::Handle<v8::Uint8Array> TextEncoder::Encode(
    v8_glue::Optional<base::string16> opt_input,
    v8_glue::Optional<TextEncodeOptions> opt_options) {
  auto const isolate = ScriptController::instance()->isolate();
  if (!opt_input.is_supplied)
    return encoder_->Encode(isolate, base::string16(), false);
  auto const is_stream = opt_options.is_supplied && opt_options.value.stream;
  return encoder_->Encode(isolate, opt_input.value, is_stream);
}

}  // namespace dom
