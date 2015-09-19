// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_EVENTS_EVENT_HANDLER_H_
#define EVITA_UI_EVENTS_EVENT_HANDLER_H_

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
 public:
  virtual ~EventHandler();

  virtual void OnEvent(Event* event);
  virtual void OnKeyEvent(KeyEvent* event);
  virtual void OnMouseEvent(MouseEvent* event);

 protected:
  EventHandler();

 private:
  DISALLOW_COPY_AND_ASSIGN(EventHandler);
};

}  // namespace ui

#endif  // EVITA_UI_EVENTS_EVENT_HANDLER_H_
