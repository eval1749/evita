// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_events_wheel_event_h)
#define INCLUDE_evita_dom_events_wheel_event_h

#include "evita/dom/events/mouse_event.h"

namespace domapi {
struct WheelEvent;
}

namespace dom {

class WheelEventInit;

namespace bindings {
class WheelEventClass;
}

class WheelEvent : public v8_glue::Scriptable<WheelEvent, MouseEvent> {
  DECLARE_SCRIPTABLE_OBJECT(WheelEvent);
  friend class bindings::WheelEventClass;

  private: int delta_mode_;
  private: double delta_x_;
  private: double delta_y_;
  private: double delta_z_;

  private: WheelEvent(const base::string16& type,
                      const WheelEventInit& init_dict);
  private: WheelEvent(const base::string16& type);
  public: explicit WheelEvent(const domapi::WheelEvent& api_event);
  public: virtual ~WheelEvent();

  private: int delta_mode() const { return delta_mode_; }
  private: double delta_x() const { return delta_x_; }
  private: double delta_y() const { return delta_y_; }
  private: double delta_z() const { return delta_z_; }

  DISALLOW_COPY_AND_ASSIGN(WheelEvent);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_events_wheel_event_h)
