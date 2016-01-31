// Copyright (c) 2013-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/events/view_event_target.h"

#include "evita/bindings/v8_glue_FocusEventInit.h"
#include "evita/dom/events/focus_event.h"
#include "evita/dom/events/keyboard_event.h"
#include "evita/dom/events/mouse_event.h"
#include "evita/dom/events/view_event_target_set.h"
#include "evita/dom/public/view_delegate.h"
#include "evita/dom/public/view_event.h"
#include "evita/dom/script_host.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// ViewEventTarget
//
ViewEventTarget::ViewEventTarget()
    : event_target_id_(ViewEventTargetSet::instance()->Register(this)) {}

ViewEventTarget::~ViewEventTarget() {}

void ViewEventTarget::ReleaseCapture() {
  ScriptHost::instance()->view_delegate()->ReleaseCapture(event_target_id());
}

void ViewEventTarget::SetCapture() {
  ScriptHost::instance()->view_delegate()->SetCapture(event_target_id());
}

std::pair<EventTarget*, FocusEvent*> ViewEventTarget::TranslateFocusEvent(
    const domapi::FocusEvent& api_event,
    EventTarget* related_target) {
  FocusEventInit event_init;
  event_init.set_related_target(related_target);
  const auto& event_type =
      api_event.event_type == domapi::EventType::Blur ? L"blur" : L"focus";
  return std::make_pair(this, new FocusEvent(event_type, event_init));
}

std::pair<EventTarget*, KeyboardEvent*> ViewEventTarget::TranslateKeyboardEvent(
    const domapi::KeyboardEvent& api_event) {
  return std::make_pair(this, new KeyboardEvent(api_event));
}

std::pair<EventTarget*, MouseEvent*> ViewEventTarget::TranslateMouseEvent(
    const domapi::MouseEvent& api_event) {
  return std::make_pair(this, new MouseEvent(api_event));
}

}  // namespace dom
