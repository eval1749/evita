// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/events/wheel_event.h"

#include "evita/bindings/v8_glue_WheelEventInit.h"
#include "evita/dom/public/view_event.h"

namespace dom {
namespace {

WheelEventInit ToWheelEventInit(const domapi::WheelEvent& event) {
  WheelEventInit init_dict;
  // EventInit
  init_dict.set_bubbles(true);
  init_dict.set_cancelable(true);

  // UiEvent
  init_dict.set_detail(0);

  // MouseEvent
  init_dict.set_alt_key(event.alt_key);
  init_dict.set_button(static_cast<int>(event.button));
  init_dict.set_buttons(event.buttons);
  init_dict.set_client_x(event.client_x);
  init_dict.set_client_y(event.client_y);
  init_dict.set_ctrl_key(event.control_key);
  init_dict.set_meta_key(false);
  init_dict.set_shift_key(event.shift_key);

  // WheelEvent
  init_dict.set_delta_mode(event.delta_mode);
  init_dict.set_delta_x(event.delta_x);
  init_dict.set_delta_y(event.delta_y);
  init_dict.set_delta_z(event.delta_z);

  return init_dict;
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// WheelEvent
//
WheelEvent::WheelEvent(const domapi::WheelEvent& event)
    : WheelEvent(L"wheel", ToWheelEventInit(event)) {}

WheelEvent::WheelEvent(const base::string16& type,
                       const WheelEventInit& init_dict)
    : ScriptableBase(type, init_dict),
      delta_mode_(init_dict.delta_mode()),
      delta_x_(init_dict.delta_x()),
      delta_y_(init_dict.delta_y()),
      delta_z_(init_dict.delta_z()) {}

WheelEvent::WheelEvent(const base::string16& type)
    : WheelEvent(type, WheelEventInit()) {}

WheelEvent::~WheelEvent() {}

}  // namespace dom
