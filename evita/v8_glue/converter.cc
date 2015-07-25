// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/v8_glue/converter.h"

#include "gin/array_buffer.h"

namespace gin {

v8::Local<v8::Value> Converter<std::vector<uint8_t>>::ToV8(
    v8::Isolate* isolate,
    const std::vector<uint8_t>& vector) {
  auto const array_buffer = v8::ArrayBuffer::New(isolate, vector.size());
  auto const data = v8::Uint8Array::New(array_buffer, 0, vector.size());
  gin::ArrayBufferView view(isolate, data);
  ::memcpy(view.bytes(), vector.data(), vector.size());
  return data;
}

bool Converter<std::vector<uint8_t>>::FromV8(v8::Isolate* isolate,
                                             v8::Local<v8::Value> val,
                                             std::vector<uint8_t>* out) {
  gin::ArrayBufferView view;
  if (!ConvertFromV8(isolate, val, &view))
    return false;
  std::vector<uint8_t> vector(view.num_bytes());
  ::memcpy(&vector[0], view.bytes(), view.num_bytes());
  *out = vector;
  return true;
}

v8::Local<v8::Value> Converter<base::char16>::ToV8(v8::Isolate* isolate,
                                                   base::char16 val) {
  return Converter<int>::ToV8(isolate, static_cast<int>(val));
}

bool Converter<base::char16>::FromV8(v8::Isolate* isolate,
                                     v8::Local<v8::Value> val,
                                     base::char16* out) {
  int int_val;
  if (!Converter<int>::FromV8(isolate, val, &int_val))
    return false;
  if (int_val < 0 || int_val > 0xFFFF)
    return false;
  *out = static_cast<base::char16>(int_val);
  return true;
}

v8::Local<v8::Value> Converter<base::string16>::ToV8(
    v8::Isolate* isolate,
    const base::string16& val) {
  return v8::String::NewFromTwoByte(
      isolate, reinterpret_cast<const uint16_t*>(val.data()),
      v8::String::kNormalString, static_cast<int>(val.length()));
}

bool Converter<base::string16>::FromV8(v8::Isolate*,
                                       v8::Local<v8::Value> val,
                                       base::string16* out) {
  if (!val->IsString())
    return false;
  auto str = v8::Local<v8::String>::Cast(val);
  auto const length = str->Length();
  out->resize(static_cast<size_t>(length));
  str->Write(reinterpret_cast<uint16_t*>(&(*out)[0]), 0, length,
             v8::String::NO_NULL_TERMINATION);
  return true;
}

v8::Local<v8::Value> Converter<v8::Local<v8::Uint8Array>>::ToV8(
    v8::Isolate*,
    v8::Local<v8::Uint8Array> value) {
  return value.As<v8::Value>();
}

v8::Local<v8::String> StringToV8(v8::Isolate* isolate,
                                 const base::string16& string) {
  return Converter<base::string16>::ToV8(isolate, string).As<v8::String>();
}

v8::Local<v8::Value> Converter<v8::Local<v8::Promise>>::ToV8(
    v8::Isolate*,
    v8::Local<v8::Promise> value) {
  return value.As<v8::Value>();
}

}  // namespace gin
