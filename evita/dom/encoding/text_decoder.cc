// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/encoding/text_decoder.h"

#include "evita/v8_glue/array_buffer_view.h"
#include "evita/v8_glue/converter.h"
#include "v8_strings.h"

namespace gin {

template<>
struct Converter<dom::TextDecoderOptions> {
  static bool FromV8(v8::Isolate* isolate,
                     v8::Handle<v8::Value> value,
                     dom::TextDecoderOptions* out) {
    if (!value->IsObject())
      return false;
    auto const options = value->ToObject();
    auto const fatal = options->Get(dom::v8Strings::fatal.Get(isolate));
    if (!ConvertFromV8(isolate, fatal, &out->fatal))
      return false;
    return true;
  }
};

template<>
struct Converter<dom::TextDecodeOptions> {
  static bool FromV8(v8::Isolate* isolate,
                     v8::Handle<v8::Value> value,
                     dom::TextDecodeOptions* out) {
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
// TextDecoderClass
//
class TextDecoderClass : public v8_glue::WrapperInfo {
  private: typedef v8_glue::WrapperInfo BaseClass;

  public: TextDecoderClass(const char* name)
      : BaseClass(name) {
  }
  public: ~TextDecoderClass() = default;

  private: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate,
        &TextDecoderClass::NewTextDecoder);
  }

  private: static TextDecoder* NewTextDecoder(
      v8_glue::Optional<base::string16> opt_label,
      v8_glue::Optional<TextDecoderOptions> opt_options) {
    TextDecoderOptions options;
    options.fatal = false;
    if (!opt_label.is_supplied)
      return new TextDecoder(L"utf-8", options);
    if (opt_options.is_supplied)
      return new TextDecoder(opt_label.value, opt_options.value);
    return new TextDecoder(opt_label.value, options);
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("encoding", &TextDecoder::encoding)
        .SetMethod("decode", &TextDecoder::Decode);
  }

  DISALLOW_COPY_AND_ASSIGN(TextDecoderClass);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TextDecoder::Decoder
//
class TextDecoder::Decoder {
  private: base::string16 encoding_;
  private: bool fatal_;

  public: Decoder(const base::string16&, bool fatal);
  public: ~Decoder();

  public: const string16& encoding() const { return encoding_; }

  public: base::string16 Decode(const void* bytes, size_t num_bytes,
                                bool is_stream);
};

TextDecoder::Decoder::Decoder(const base::string16&, bool fatal)
    : encoding_(L"utf-8"), fatal_(fatal) {
}

TextDecoder::Decoder::~Decoder() {
}

base::string16 TextDecoder::Decoder::Decode(const void* bytes, size_t num_bytes,
                                            bool) {
  auto const start = reinterpret_cast<const uint8_t*>(bytes);
  auto const end = start + num_bytes;
  base::string16 text(num_bytes, static_cast<base::char16>('?'));
  for (auto runner = start; runner < end; ++runner)
    text[static_cast<size_t>(runner - start)] = *runner;
  return std::move(text);
}

//////////////////////////////////////////////////////////////////////
//
// TextDecoder
//
DEFINE_SCRIPTABLE_OBJECT(TextDecoder, TextDecoderClass);

TextDecoder::TextDecoder(const base::string16& label,
                         const TextDecoderOptions& options)
    : decoder_(new Decoder(label, options.fatal)) {
}

TextDecoder::~TextDecoder() {
}

const base::string16& TextDecoder::encoding() const {
  return decoder_->encoding();
}

base::string16 TextDecoder::Decode(
    v8_glue::Optional<gin::ArrayBufferView*> opt_input,
    v8_glue::Optional<TextDecodeOptions> opt_options) {
  if (!opt_input.is_supplied)
    return decoder_->Decode(nullptr, 0, false);
  auto const input = opt_input.value;
  auto const is_stream = opt_options.is_supplied && opt_options.value.stream;
  return decoder_->Decode(input->bytes(), input->num_bytes(), is_stream);
}

}  // namespace dom
