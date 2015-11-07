// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/events/event_source.h"

#include "base/logging.h"
#include "base/trace_event/trace_event.h"
#include "evita/dom/public/text_composition_event.h"
#include "evita/dom/public/view_event.h"
#include "evita/dom/public/view_event_handler.h"
#include "evita/editor/application.h"
#include "evita/ui/base/ime/text_composition.h"
#include "evita/ui/events/event.h"

namespace views {

namespace {

int event_id_counter;

int next_event_id() {
  return ++event_id_counter;
}

domapi::ViewEventHandler* view_event_handler() {
  return editor::Application::instance()->view_event_handler();
}

domapi::EventType ConvertEventType(const ui::KeyEvent event) {
  auto const event_type = event.event_type();
  if (event_type == ui::EventType::KeyPressed)
    return domapi::EventType::KeyDown;

  if (event_type == ui::EventType::KeyReleased)
    return domapi::EventType::KeyUp;

  return domapi::EventType::Invalid;
}

domapi::EventType ConvertEventType(const ui::MouseEvent event) {
  auto const event_type = event.event_type();
  if (event_type == ui::EventType::MousePressed) {
    if (!event.click_count())
      return domapi::EventType::MouseDown;
    if (event.click_count() == 1)
      return domapi::EventType::Click;
    return domapi::EventType::DblClick;
  }

  if (event_type == ui::EventType::MouseReleased)
    return domapi::EventType::MouseUp;

  if (event_type == ui::EventType::MouseMoved)
    return domapi::EventType::MouseMove;

  if (event_type == ui::EventType::MouseEntered)
    return domapi::EventType::MouseEnter;

  if (event_type == ui::EventType::MouseExited)
    return domapi::EventType::MouseLeave;

  if (event_type == ui::EventType::MouseWheel)
    return domapi::EventType::Wheel;

  return domapi::EventType::Invalid;
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// EventSource
//
EventSource::EventSource(domapi::EventTargetId event_target_id)
    : event_target_id_(event_target_id) {
  DCHECK_NE(domapi::kInvalidEventTargetId, event_target_id_);
}

EventSource::~EventSource() {}

void EventSource::DispatchFocusEvent(domapi::EventType event_type,
                                     domapi::EventTargetId related_target_id) {
  TRACE_EVENT0("views", "EventSource::DispatchFocusEvent");
  DCHECK(event_type == domapi::EventType::Blur ||
         event_type == domapi::EventType::Focus);
  domapi::FocusEvent api_event;
  api_event.event_id = next_event_id();
  api_event.event_type = event_type;
  api_event.related_target_id = related_target_id;
  api_event.target_id = event_target_id_;
  view_event_handler()->DispatchFocusEvent(api_event);
}

void EventSource::DispatchKeyboardEvent(const ui::KeyEvent& event) {
  TRACE_EVENT0("views", "EventSource::DispatchKeyboardEvent");
  domapi::KeyboardEvent api_event;
  api_event.event_id = event.id();
  api_event.alt_key = event.alt_key();
  api_event.control_key = event.control_key();
  api_event.event_type = ConvertEventType(event);
  api_event.key_code = event.raw_key_code();
  api_event.meta_key = false;
  api_event.repeat = event.repeat();
  api_event.shift_key = event.shift_key();
  api_event.target_id = event_target_id_;
  view_event_handler()->DispatchKeyboardEvent(api_event);
  TRACE_EVENT_ASYNC_END2("input", "KeyEvent", event.id(), "type",
                         static_cast<int>(event.event_type()), "key_code",
                         event.raw_key_code());
}

void EventSource::DispatchMouseEvent(const ui::MouseEvent& event) {
#define MUST_EQUAL(name)                                       \
  static_assert(static_cast<int>(domapi::MouseButton::name) == \
                    static_cast<int>(ui::MouseButton::name),   \
                "Button name " #name " must be equal.")
  MUST_EQUAL(Left);
  MUST_EQUAL(Middle);
  MUST_EQUAL(Right);
  MUST_EQUAL(Other1);
  MUST_EQUAL(Other2);

  domapi::MouseEvent api_event;
  api_event.event_id = event.id();
  api_event.alt_key = event.alt_key();
  api_event.button = static_cast<domapi::MouseButton>(event.button());
  api_event.buttons = event.buttons();
  api_event.client_x = event.location().x();
  api_event.client_y = event.location().y();
  api_event.control_key = event.control_key();
  api_event.event_type = ConvertEventType(event);
  api_event.shift_key = event.shift_key();
  api_event.target_id = event_target_id_;
  view_event_handler()->DispatchMouseEvent(api_event);
}

void EventSource::DispatchTextCompositionEvent(
    domapi::EventType event_type,
    const ui::TextComposition& composition) {
  TRACE_EVENT0("views", "EventSource::DispatchTextCompositionEvent");
  domapi::TextCompositionData data;
  data.caret = composition.caret();
  data.text = composition.text();
  for (auto span : composition.spans()) {
    domapi::TextCompositionSpan api_span;
    api_span.start = span.start;
    api_span.end = span.end;
    api_span.data = static_cast<int>(span.type);
    data.spans.push_back(api_span);
  }

  domapi::TextCompositionEvent api_event;
  api_event.event_id = next_event_id();
  api_event.event_type = event_type;
  api_event.target_id = event_target_id_;
  api_event.data = data;
  view_event_handler()->DispatchTextCompositionEvent(api_event);
}

void EventSource::DispatchWheelEvent(const ui::MouseWheelEvent& event) {
  TRACE_EVENT0("views", "EventSource::DispatchWheelEvent");
  domapi::WheelEvent api_event;
  api_event.event_id = event.id();
  api_event.alt_key = event.alt_key();
  api_event.button = static_cast<domapi::MouseButton>(event.button());
  api_event.buttons = event.buttons();
  api_event.client_x = event.location().x();
  api_event.client_y = event.location().y();
  api_event.control_key = event.control_key();
  api_event.event_type = ConvertEventType(event);
  api_event.shift_key = event.shift_key();
  api_event.target_id = event_target_id_;
  api_event.delta_mode = 0;
  api_event.delta_x = 0.0;
  api_event.delta_y = event.delta();
  api_event.delta_z = 0.0;
  view_event_handler()->DispatchWheelEvent(api_event);
}

}  // namespace views
