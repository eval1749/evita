// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/mouse_event_init.h"

#include "evita/dom/converter.h"
#include "evita/dom/events/event_target.h"
#include "evita/v8_glue/nullable.h"
#include "v8_strings.h"

namespace dom {

namespace {
int ConvertClickCount(const domapi::MouseEvent& event) {
  if (event.event_type == domapi::EventType::Click)
    return 1;
  if (event.event_type == domapi::EventType::DblClick)
    return 2;
  return 0;
}
}  // namespace

MouseEventInit::MouseEventInit(const domapi::MouseEvent& event)
    : alt_key_(event.alt_key), button_(static_cast<int>(event.button)),
      buttons_(event.buttons), client_x_(event.client_x),
      client_y_(event.client_y), ctrl_key_(event.control_key),
      meta_key_(false), shift_key_(event.shift_key) {
  set_bubbles(true);
  set_cancelable(true);
  set_detail(ConvertClickCount(event));
}

MouseEventInit::MouseEventInit()
    : alt_key_(false), button_(0), buttons_(0), client_x_(0), client_y_(0),
      ctrl_key_(false), meta_key_(false), shift_key_(false) {
  set_bubbles(true);
  set_cancelable(true);
}

MouseEventInit::~MouseEventInit() {
}

Dictionary::HandleResult MouseEventInit::HandleKeyValue(
    v8::Handle<v8::Value> key, v8::Handle<v8::Value> value) {

  if (key->Equals(v8Strings::altKey.Get(isolate()))) {
    return gin::ConvertFromV8(isolate(), value, &alt_key_) ?
        HandleResult::Succeeded : HandleResult::CanNotConvert;
  }
  if (key->Equals(v8Strings::button.Get(isolate()))) {
    return gin::ConvertFromV8(isolate(), value, &button_) ?
        HandleResult::Succeeded : HandleResult::CanNotConvert;
  }
  if (key->Equals(v8Strings::buttons.Get(isolate()))) {
    return gin::ConvertFromV8(isolate(), value, &buttons_) ?
        HandleResult::Succeeded : HandleResult::CanNotConvert;
  }
  if (key->Equals(v8Strings::clientX.Get(isolate()))) {
    return gin::ConvertFromV8(isolate(), value, &client_x_) ?
        HandleResult::Succeeded : HandleResult::CanNotConvert;
  }
  if (key->Equals(v8Strings::clientY.Get(isolate()))) {
    return gin::ConvertFromV8(isolate(), value, &client_y_) ?
        HandleResult::Succeeded : HandleResult::CanNotConvert;
  }
  if (key->Equals(v8Strings::ctrlKey.Get(isolate()))) {
    return gin::ConvertFromV8(isolate(), value, &ctrl_key_) ?
        HandleResult::Succeeded : HandleResult::CanNotConvert;
  }
  if (key->Equals(v8Strings::shiftKey.Get(isolate()))) {
    return gin::ConvertFromV8(isolate(), value, &shift_key_) ?
        HandleResult::Succeeded : HandleResult::CanNotConvert;
  }

  return UiEventInit::HandleKeyValue(key, value);
}

}  // namespace dom
