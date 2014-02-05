// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/form_event_init.h"

#include "evita/dom/converter.h"
#include "evita/v8_glue/nullable.h"
#include "v8_strings.h"

namespace dom {

FormEventInit::FormEventInit() {
}

FormEventInit::~FormEventInit() {
}

InitDict::HandleResult FormEventInit::HandleKeyValue(
    v8::Handle<v8::Value> key, v8::Handle<v8::Value> value) {
  if (key->Equals(v8Strings::data.Get(isolate()))) {
    return gin::ConvertFromV8(isolate(), value, &data_) ?
        HandleResult::Succeeded : HandleResult::CanNotConvert;
  }

  return EventInit::HandleKeyValue(key, value);
}

}  // namespace dom
