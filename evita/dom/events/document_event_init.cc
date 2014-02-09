// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/document_event_init.h"

#include "evita/dom/converter.h"
#include "v8_strings.h"

namespace dom {

DocumentEventInit::DocumentEventInit() : error_code_(0) {
}

DocumentEventInit::~DocumentEventInit() {
}

InitDict::HandleResult DocumentEventInit::HandleKeyValue(
    v8::Handle<v8::Value> key, v8::Handle<v8::Value> value) {
  if (key->Equals(v8Strings::errorCode.Get(isolate()))) {
    return gin::ConvertFromV8(isolate(), value, &error_code_) ?
        HandleResult::Succeeded : HandleResult::CanNotConvert;
  }

  return EventInit::HandleKeyValue(key, value);
}

}  // namespace dom
