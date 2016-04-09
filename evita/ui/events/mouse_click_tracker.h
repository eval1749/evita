// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_EVENTS_MOUSE_CLICK_TRACKER_H_
#define EVITA_UI_EVENTS_MOUSE_CLICK_TRACKER_H_

#include "evita/ui/events/event.h"

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// MouseClickTracker tracks mouse click.
//
class MouseClickTracker final {
 public:
  MouseClickTracker();
  ~MouseClickTracker();

  int click_count() const { return click_count_; }

  static MouseClickTracker* GetInstance();

  void OnMousePressed(const MouseEvent& event);
  void OnMouseReleased(const MouseEvent& event);

 private:
  enum class State;

  bool is_repeated_event(const MouseEvent& event) const;
  bool is_same_location(const MouseEvent& event) const;
  bool is_same_time(const MouseEvent& event) const;

  int click_count_ = 0;
  MouseEvent last_event_;
  State state_;

  DISALLOW_COPY_AND_ASSIGN(MouseClickTracker);
};

}  // namespace ui

#endif  // EVITA_UI_EVENTS_MOUSE_CLICK_TRACKER_H_
