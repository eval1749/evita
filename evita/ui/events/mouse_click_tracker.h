// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_events_mouse_click_tracker_h)
#define INCLUDE_evita_ui_events_mouse_click_tracker_h

#include "common/memory/singleton.h"
#include "evita/ui/events/event.h"

namespace ui {

// Tracking mouse click.
class MouseClickTracker : public common::Singleton<MouseClickTracker> {
  DECLARE_SINGLETON_CLASS(MouseClickTracker);

  // These values match the Windows defaults.
  private: static const auto kDoubleClickTimeMS = 500;
  private: static const auto kDoubleClickWidth = 4;
  private: static const auto kDoubleClickHeight = 4;

  private: enum class State {
    Start,
    Pressed,
    PressedReleased,
    PressedReleasedPressed,
  };

  private: int click_count_;
  private: MouseEvent last_event_;
  private: State state_;

  private: MouseClickTracker();
  public: ~MouseClickTracker();

  public: int click_count() const { return click_count_; }
  private: bool is_repeated_event(const MouseEvent& event) const;
  private: bool is_same_location(const MouseEvent& event) const;
  private: bool is_same_time(const MouseEvent& event) const;

  public: void OnMousePressed(const MouseEvent& event);
  public: void OnMouseReleased(const MouseEvent& event);

  DISALLOW_COPY_AND_ASSIGN(MouseClickTracker);
};

}  // namespace ui

#endif //!defined(INCLUDE_evita_ui_events_mouse_click_tracker_h)
