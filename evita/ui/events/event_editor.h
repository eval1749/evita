// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_EVENTS_EVENT_EDITOR_H_
#define EVITA_UI_EVENTS_EVENT_EDITOR_H_

#include "base/macros.h"
#include "evita/gfx/rect.h"

namespace ui {

class LocatedEvent;
class MouseEvent;

//////////////////////////////////////////////////////////////////////
//
// EventEditor
//
class EventEditor final {
 public:
  EventEditor();
  ~EventEditor();

  void SetClickCount(MouseEvent* event, int count);
  void SetClientPoint(LocatedEvent* event, const gfx::Point& client_point);

 private:
  DISALLOW_COPY_AND_ASSIGN(EventEditor);
};

}  // namespace ui

#endif  // EVITA_UI_EVENTS_EVENT_EDITOR_H_
