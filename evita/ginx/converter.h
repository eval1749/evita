// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_GINX_CONVERTER_H_
#define EVITA_GINX_CONVERTER_H_

#include <string>
#include <vector>

#include "base/strings/string16.h"
#include "evita/ginx/v8.h"
BEGIN_V8_INCLUDE
#include "gin/converter.h"
END_V8_INCLUDE

namespace gin {

// std::vecotr<uint_8>
template <>
struct Converter<std::vector<uint8_t>> {
  static v8::Local<v8::Value> ToV8(v8::Isolate* isolate,
                                   const std::vector<uint8_t>& vector);
  static bool FromV8(v8::Isolate* isolate,
                     v8::Local<v8::Value> val,
                     std::vector<uint8_t>* out);
};

template <>
struct ToV8ReturnsMaybe<std::vector<uint8_t>> {
  static const bool value = false;
};

template <>
struct Converter<base::char16> {
  static v8::Local<v8::Value> ToV8(v8::Isolate* isolate, base::char16 val);
  static bool FromV8(v8::Isolate* isolate,
                     v8::Local<v8::Value> val,
                     base::char16* out);
};

template <>
struct Converter<base::string16> {
  static v8::Local<v8::Value> ToV8(v8::Isolate* isolate,
                                   const base::string16& string);
  static bool FromV8(v8::Isolate* isolate,
                     v8::Local<v8::Value> val,
                     base::string16* out);
};

template <>
struct Converter<v8::Local<v8::Uint8Array>> {
  static v8::Local<v8::Value> ToV8(v8::Isolate* isolate,
                                   v8::Local<v8::Uint8Array> value);
};

template <typename T>
struct Converter<v8::Maybe<T>> {
  static v8::Local<v8::Value> ToV8(v8::Isolate* isolate,
                                   const v8::Maybe<T>& maybe) {
    return maybe.has_value ? ConvertToV8(isolate, maybe.value)
                           : v8::Null(isolate);
  }
  static bool FromV8(v8::Isolate* isolate,
                     v8::Local<v8::Value> val,
                     v8::Maybe<T>* out) {
    if (val.IsNull()) {
      *out = nullptr;
      return true;
    }
    return Converter<T>::FromV8(isolate, val, out);
  }
};

// v8::Promise
template <>
struct Converter<v8::Local<v8::Promise>> {
  static v8::Local<v8::Value> ToV8(v8::Isolate* isolate,
                                   v8::Local<v8::Promise> promise);
};

// Converter for enum:bool types as int.
// Note: I'm not sure how to get base type of |enum|.
template <typename T>
struct Converter<
    T,
    typename std::enable_if<std::is_enum<T>::value && sizeof(T) == 1>::type> {
  static bool FromV8(v8::Isolate* isolate, v8::Local<v8::Value> value, T* out) {
    bool bool_value = false;
    if (!ConvertFromV8(isolate, value, &bool_value))
      return false;
    *out = static_cast<T>(bool_value);
    return true;
  }
  static v8::Local<v8::Value> ToV8(v8::Isolate* isolate, T value) {
    return ConvertToV8(isolate, static_cast<bool>(value));
  }
};

// Converter for enum:<non bool> types as int.
// Note: I'm not sure how to get base type of |enum|.
template <typename T>
struct Converter<
    T,
    typename std::enable_if<std::is_enum<T>::value && sizeof(T) >= 2>::type> {
  static bool FromV8(v8::Isolate* isolate, v8::Local<v8::Value> value, T* out) {
    int int_value = 0;
    if (!ConvertFromV8(isolate, value, &int_value))
      return false;
    *out = static_cast<T>(int_value);
    return true;
  }
  static v8::Local<v8::Value> ToV8(v8::Isolate* isolate, T value) {
    return ConvertToV8(isolate, static_cast<int>(value));
  }
};

v8::Local<v8::String> StringToV8(v8::Isolate* isolate,
                                 const base::string16& string);

}  // namespace gin

#endif  // EVITA_GINX_CONVERTER_H_
