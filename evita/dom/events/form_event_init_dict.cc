// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/form_event_init_dict.h"

#include "evita/dom/converter.h"
#include "evita/v8_glue/nullable.h"
#include "v8_strings.h"

namespace dom {

FormEventInitDict::FormEventInitDict() {
}

FormEventInitDict::~FormEventInitDict() {
}

InitDict::HandleResult FormEventInitDict::HandleKeyValue(
    v8::Handle<v8::String> key, v8::Handle<v8::Value> value) {
  if (EqualNames(key, v8Strings::data)) {
    return gin::ConvertFromV8(isolate(), value, &data_) ?
        HandleResult::Succeeded : HandleResult::CanNotConvert;
  }

  return EventInitDict::HandleKeyValue(key, value);
}

}  // namespace dom
