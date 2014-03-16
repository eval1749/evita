// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/text/mutation_observer_init.h"

#include "evita/dom/converter.h"
#include "v8_strings.h"

namespace dom {

MutationObserverInit::MutationObserverInit() : summary_(false) {
}

MutationObserverInit::~MutationObserverInit() {
}

Dictionary::HandleResult MutationObserverInit::HandleKeyValue(
    v8::Handle<v8::Value> key, v8::Handle<v8::Value> value) {
  if (key->Equals(v8Strings::summary.Get(isolate()))) {
    return gin::ConvertFromV8(isolate(), value, &summary_) ?
        HandleResult::Succeeded : HandleResult::CanNotConvert;
  }
  return HandleResult::NotFound;
}

}  // namespace dom
