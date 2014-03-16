// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_dictionary_h)
#define INCLUDE_evita_dom_dictionary_h

#include "base/basictypes.h"
#include "evita/v8_glue/converter.h"

namespace dom {

class Dictionary {
  protected: enum class HandleResult {
    NotFound,
    Succeeded,
    CanNotConvert,
  };

  protected: Dictionary();
  public: virtual ~Dictionary();

  protected: v8::Isolate* isolate() const;

  protected: virtual HandleResult HandleKeyValue(
      v8::Handle<v8::Value> key, v8::Handle<v8::Value> value) = 0;
  public: bool Init(v8::Isolate* isolate, v8::Handle<v8::Value> dict);
};

}  // namespace dom

namespace gin {
template<typename T>
struct Converter<T, typename
    std::enable_if<std::is_base_of<dom::Dictionary, T>::value>::type>
{
  static bool FromV8(v8::Isolate* isolate, v8::Handle<v8::Value> val,
                     T* out) {
    return out->Init(isolate, val->ToObject());
  }
};
}  // namespace gin

#endif //!defined(INCLUDE_evita_dom_dictionary_h)
