// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_event_source_h)
#define INCLUDE_evita_views_event_source_h

#include "evita/dom/public/event_target_id.h"

namespace domapi {
enum class EventType;
struct TextCompositionData;
}

namespace ui {
class KeyboardEvent;
class MouseEvent;
class MouseWheelEvent;
}

namespace views {

class EventSource {
  private: const domapi::EventTargetId event_target_id_;

  protected: explicit EventSource(domapi::EventTargetId event_target_id);
  public: ~EventSource();

  protected: domapi::EventTargetId event_target_id() const {
    return event_target_id_;
  }

  public: void DispatchFocusEvent(domapi::EventType event_type,
                                  domapi::EventTargetId related_target_id);
  public: void DispatchKeyboardEvent(const ui::KeyboardEvent& event);
  public: void DispatchMouseEvent(const ui::MouseEvent& event);
  public: void DispatchTxetCompositionEvent(
      domapi::EventType event_type, const domapi::TextCompositionData& data);
  public: void DispatchWheelEvent(const ui::MouseWheelEvent& event);

  DISALLOW_COPY_AND_ASSIGN(EventSource);
};

}  // namespace views

#endif //!defined(INCLUDE_evita_views_event_source_h)
