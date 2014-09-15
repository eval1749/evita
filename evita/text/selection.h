// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_text_selection_h)
#define INCLUDE_evita_text_selection_h

#include "evita/css/style.h"

namespace text {
class Range;
class Buffer;

class Selection {
  private: Range* range_;
  private: bool start_is_active_;

  public: Selection(Range* range);
  public: ~Selection();

  public: Posn anchor_offset() const;
  public: Buffer* buffer() const;
  public: Posn end() const;
  public: Posn focus_offset() const;
  public: Posn start() const;

  public: bool IsStartActive() const { return start_is_active_; }
  public: void SetStartIsActive(bool new_start_is_active) {
    start_is_active_ = new_start_is_active;
  }

  DISALLOW_COPY_AND_ASSIGN(Selection);
};

}  // namespace text

#endif //!defined(INCLUDE_evita_text_selection_h)
