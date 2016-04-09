// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/events/event_editor.h"

#include "evita/ui/events/event.h"

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// EventEditor
//
EventEditor::EventEditor() {}
EventEditor::~EventEditor() {}

void EventEditor::SetClickCount(MouseEvent* event, int count) {
  DCHECK(event->type() == EventType::MousePressed ||
         event->type() == EventType::MouseReleased);
  DCHECK_GE(count, 0);
  DCHECK_LE(count, 2);
  event->click_count_ = count;
}

}  // namespace ui
