// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/events/wheel_event_init.h"

#include "evita/dom/converter.h"
#include "evita/dom/events/event_target.h"
#include "evita/dom/public/view_event.h"
#include "evita/v8_glue/nullable.h"
#include "v8_strings.h"

namespace dom {

WheelEventInit::WheelEventInit()
    : delta_mode_(0), delta_x_(0.0), delta_y_(0.0), delta_z_(0.0) {
  set_bubbles(true);
  set_cancelable(true);
}

WheelEventInit::~WheelEventInit() {
}

Dictionary::HandleResult WheelEventInit::HandleKeyValue(
    v8::Handle<v8::Value> key, v8::Handle<v8::Value> value) {
  if (key->Equals(v8Strings::deltaMode.Get(isolate()))) {
    return gin::ConvertFromV8(isolate(), value, &delta_mode_) ?
        HandleResult::Succeeded : HandleResult::CanNotConvert;
  }
  if (key->Equals(v8Strings::deltaX.Get(isolate()))) {
    return gin::ConvertFromV8(isolate(), value, &delta_x_) ?
        HandleResult::Succeeded : HandleResult::CanNotConvert;
  }
  if (key->Equals(v8Strings::deltaY.Get(isolate()))) {
    return gin::ConvertFromV8(isolate(), value, &delta_y_) ?
        HandleResult::Succeeded : HandleResult::CanNotConvert;
  }
  if (key->Equals(v8Strings::deltaZ.Get(isolate()))) {
    return gin::ConvertFromV8(isolate(), value, &delta_z_) ?
        HandleResult::Succeeded : HandleResult::CanNotConvert;
  }
  return UiEventInit::HandleKeyValue(key, value);
}

}  // namespace dom
