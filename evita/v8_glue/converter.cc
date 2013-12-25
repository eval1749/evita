// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/v8_glue/converter.h"

namespace gin {

using v8::Handle;
using v8::Isolate;
using v8::String;
using v8::Value;

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

}  // namespace gin
