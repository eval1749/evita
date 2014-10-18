// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_events_event_handler_h)
#define INCLUDE_evita_ui_events_event_handler_h

#include "base/macros.h"

namespace ui {

class Event;
class KeyEvent;
class MouseEvent;

//////////////////////////////////////////////////////////////////////
//
// EventHandler
//
class EventHandler {
  protected: EventHandler();
  public: virtual ~EventHandler();

  public: virtual void OnEvent(Event* event);
  public: virtual void OnKeyEvent(KeyEvent* event);
  public: virtual void OnMouseEvent(MouseEvent* event);

  DISALLOW_COPY_AND_ASSIGN(EventHandler);
};

}  // namespace ui

#endif // !defined(INCLUDE_evita_ui_events_event_handler_h)
