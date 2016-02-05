// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_EVENTS_WHEEL_EVENT_H_
#define EVITA_DOM_EVENTS_WHEEL_EVENT_H_

#include "evita/dom/events/mouse_event.h"

namespace domapi {
struct WheelEvent;
}

namespace dom {

class WheelEventInit;

namespace bindings {
class WheelEventClass;
}

class WheelEvent final : public ginx::Scriptable<WheelEvent, MouseEvent> {
  DECLARE_SCRIPTABLE_OBJECT(WheelEvent);

 public:
  explicit WheelEvent(const domapi::WheelEvent& api_event);
  ~WheelEvent() final;

 private:
  friend class bindings::WheelEventClass;

  WheelEvent(const base::string16& type, const WheelEventInit& init_dict);
  explicit WheelEvent(const base::string16& type);

  int delta_mode() const { return delta_mode_; }
  double delta_x() const { return delta_x_; }
  double delta_y() const { return delta_y_; }
  double delta_z() const { return delta_z_; }

  int delta_mode_;
  double delta_x_;
  double delta_y_;
  double delta_z_;

  DISALLOW_COPY_AND_ASSIGN(WheelEvent);
};

}  // namespace dom

#endif  // EVITA_DOM_EVENTS_WHEEL_EVENT_H_
