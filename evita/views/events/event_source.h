// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_EVENTS_EVENT_SOURCE_H_
#define EVITA_VIEWS_EVENTS_EVENT_SOURCE_H_

#include "base/macros.h"
#include "evita/dom/public/event_target_id.h"

namespace domapi {
enum class EventType;
struct TextCompositionData;
}

namespace ui {
class KeyEvent;
class MouseEvent;
class MouseWheelEvent;
class TextComposition;
}

namespace views {

class EventSource {
 public:
  ~EventSource();

  void DispatchFocusEvent(domapi::EventType event_type,
                                  domapi::EventTargetId related_target_id);
  void DispatchKeyboardEvent(const ui::KeyEvent& event);
  void DispatchMouseEvent(const ui::MouseEvent& event);
  void DispatchTextCompositionEvent(
      domapi::EventType event_type, const ui::TextComposition& composition);
  void DispatchWheelEvent(const ui::MouseWheelEvent& event);

 protected:
  explicit EventSource(domapi::EventTargetId event_target_id);
  domapi::EventTargetId event_target_id() const { return event_target_id_; }

 private:
  const domapi::EventTargetId event_target_id_;

  DISALLOW_COPY_AND_ASSIGN(EventSource);
};

}  // namespace views

#endif  // EVITA_VIEWS_EVENTS_EVENT_SOURCE_H_
