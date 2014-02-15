// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/regexp_init.h"

#include "evita/dom/converter.h"
#include "v8_strings.h"

namespace dom {

RegExpInit::RegExpInit()
    : backward_(false), global_(false), ignore_case_(false),
      match_exact_(false), match_word_(false), multiline_(false),
      sticky_(false) {
}

RegExpInit::~RegExpInit() {
}

InitDict::HandleResult RegExpInit::HandleKeyValue(
    v8::Handle<v8::Value> key, v8::Handle<v8::Value> value) {
  if (key->Equals(v8Strings::backward.Get(isolate()))) {
    return gin::ConvertFromV8(isolate(), value, &backward_) ?
        HandleResult::Succeeded : HandleResult::CanNotConvert;
  }
  if (key->Equals(v8Strings::global.Get(isolate()))) {
    return gin::ConvertFromV8(isolate(), value, &global_) ?
        HandleResult::Succeeded : HandleResult::CanNotConvert;
  }
  if (key->Equals(v8Strings::ignoreCase.Get(isolate()))) {
    return gin::ConvertFromV8(isolate(), value, &ignore_case_) ?
        HandleResult::Succeeded : HandleResult::CanNotConvert;
  }
  if (key->Equals(v8Strings::matchExact.Get(isolate()))) {
    return gin::ConvertFromV8(isolate(), value, &match_exact_) ?
        HandleResult::Succeeded : HandleResult::CanNotConvert;
  }
  if (key->Equals(v8Strings::matchWord.Get(isolate()))) {
    return gin::ConvertFromV8(isolate(), value, &match_word_) ?
        HandleResult::Succeeded : HandleResult::CanNotConvert;
  }
  if (key->Equals(v8Strings::multiline.Get(isolate()))) {
    return gin::ConvertFromV8(isolate(), value, &multiline_) ?
        HandleResult::Succeeded : HandleResult::CanNotConvert;
  }
  if (key->Equals(v8Strings::sticky.Get(isolate()))) {
    return gin::ConvertFromV8(isolate(), value, &sticky_) ?
        HandleResult::Succeeded : HandleResult::CanNotConvert;
  }
  return HandleResult::NotFound;
}

}  // namespace dom
