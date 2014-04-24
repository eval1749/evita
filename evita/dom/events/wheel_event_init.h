// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_events_mouse_wheel_event_init_h)
#define INCLUDE_evita_dom_events_mouse_wheel_event_init_h

#include "evita/bindings/MouseEventInit.h"

namespace domapi {
struct WheelEvent;
}

namespace dom {

class EventTarget;

class WheelEventInit : public MouseEventInit {
  private: int delta_mode_;
  private: double delta_x_;
  private: double delta_y_;
  private: double delta_z_;

  public: WheelEventInit();
  public: virtual ~WheelEventInit();

  public: int delta_mode() const { return delta_mode_; }
  public: void set_delta_mode(int new_delta_mode) {
    delta_mode_ = new_delta_mode;
  }
  public: double delta_x() const { return delta_x_; }
  public: void set_delta_x(double new_delta_x) {
    delta_x_ = new_delta_x;
  }
  public: double delta_y() const { return delta_y_; }
  public: void set_delta_y(double new_delta_y) {
    delta_y_ = new_delta_y;
  }
  public: double delta_z() const { return delta_z_; }
  public: void set_delta_z(double new_delta_z) {
    delta_z_ = new_delta_z;
  }

  // dom::Dictionary
  private: virtual HandleResult HandleKeyValue(
      v8::Handle<v8::Value> key, v8::Handle<v8::Value> value) override;
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_events_mouse_wheel_event_init_h)
