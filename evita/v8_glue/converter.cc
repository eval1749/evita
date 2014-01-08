// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/v8_glue/converter.h"

namespace gin {

using v8::Handle;
using v8::Isolate;
using v8::String;
using v8::Value;

Handle<Value> Converter<base::char16>::ToV8(Isolate* isolate,
                                              base::char16 val) {
  return Converter<int>::ToV8(isolate, static_cast<int>(val));
}

bool Converter<base::char16>::FromV8(Isolate* isolate, Handle<Value> val,
                                       base::char16* out) {
  int int_val;
  if (!Converter<int>::FromV8(isolate, val, &int_val))
    return false;
  if (int_val < 0 || int_val > 0xFFFF)
    return false;
  *out = static_cast<base::char16>(int_val);
  return true;
}

Handle<Value> Converter<base::string16>::ToV8(Isolate* isolate,
                                              const base::string16& val) {
  return String::NewFromTwoByte(
      isolate, reinterpret_cast<const uint16_t*>(val.data()),
      String::kNormalString, val.length());
}

bool Converter<base::string16>::FromV8(Isolate*, Handle<Value> val,
                                       base::string16* out) {
  if (!val->IsString())
    return false;
  auto str = Handle<String>::Cast(val);
  auto const length = str->Length();
  out->resize(length);
  str->Write(reinterpret_cast<uint16_t*>(&(*out)[0]), 0, length,
             String::NO_NULL_TERMINATION);
  return true;
}

Handle<Value> Converter<text::Posn>::ToV8(Isolate* isolate,
                                          text::Posn position) {
  return Converter<int>::ToV8(isolate, static_cast<int>(position));
}

bool Converter<text::Posn>::FromV8(Isolate* isolate, Handle<Value> val,
                                       text::Posn* out) {
  int int_val;
  if (!Converter<int>::FromV8(isolate, val, &int_val))
    return false;
  *out = text::Posn(int_val);
  return true;
}

Handle<Value> StringToV8(v8::Isolate* isolate,
                         const base::string16& string) {
  return Converter<base::string16>::ToV8(isolate, string);
}

}  // namespace gin
