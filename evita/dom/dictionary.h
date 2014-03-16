// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_dictionary_h)
#define INCLUDE_evita_dom_dictionary_h

#include "base/basictypes.h"
#include "evita/v8_glue/v8.h"

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
  public: bool Init(v8::Handle<v8::Object> dict);

  DISALLOW_COPY_AND_ASSIGN(Dictionary);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_dictionary_h)
