// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_DICTIONARY_H_
#define EVITA_DOM_DICTIONARY_H_

#include "base/basictypes.h"
// TODO(eval1749): We should include "dom/time_stamp.h" here, instead of
// IDL compiler should insert include directive for generated files. This is
// workaround for "dom/os/DirectoryEntry.idl".
#include "evita/dom/time_stamp.h"
#include "evita/v8_glue/converter.h"

namespace dom {

class Dictionary {
 public:
  virtual ~Dictionary();

  bool Init(v8::Isolate* isolate, v8::Handle<v8::Value> dict);

 protected:
  enum class HandleResult {
    NotFound,
    Succeeded,
    CanNotConvert,
  };

  Dictionary();

  v8::Isolate* isolate() const;

  virtual HandleResult HandleKeyValue(v8::Handle<v8::Value> key,
                                      v8::Handle<v8::Value> value) = 0;
};

}  // namespace dom

namespace gin {
template <typename T>
struct Converter<
    T,
    typename std::enable_if<std::is_base_of<dom::Dictionary, T>::value>::type> {
  static bool FromV8(v8::Isolate* isolate, v8::Handle<v8::Value> val, T* out) {
    return out->Init(isolate, val->ToObject());
  }
};
}  // namespace gin

#endif  // EVITA_DOM_DICTIONARY_H_
