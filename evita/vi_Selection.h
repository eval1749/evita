// Copyright (C) 1996-2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_listener_winapp_visual_selection_h)
#define INCLUDE_listener_winapp_visual_selection_h

#include "evita/css/style.h"

class TextEditWindow;

namespace text {
class Range;
class Buffer;
}

class Selection {
  private: text::Range* range_;
  private: bool start_is_active_;

  public: Selection(text::Range* range);
  public: ~Selection();

  public: text::Posn anchor_offset() const;
  public: text::Posn end() const;
  public: text::Posn focus_offset() const;
  public: text::Posn start() const;

  public: bool IsStartActive() const { return start_is_active_; }
  public: void SetStartIsActive(bool new_start_is_active) {
    start_is_active_ = new_start_is_active;
  }

  DISALLOW_COPY_AND_ASSIGN(Selection);
};

#endif //!defined(INCLUDE_listener_winapp_visual_selection_h)
