// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_LAYOUT_LAYOUT_CARET_H_
#define EVITA_LAYOUT_LAYOUT_CARET_H_

#include "base/macros.h"
#include "evita/gfx/forward.h"
#include "evita/ui/caret.h"

namespace views {

//////////////////////////////////////////////////////////////////////
//
// LayoutCaret
//
class LayoutCaret final {
 public:
  enum class State {
    Hide,
    None,
    Show,
  };

  LayoutCaret(State state, const gfx::RectF& bounds);
  ~LayoutCaret();

  const gfx::RectF& bounds() const { return bounds_; }

  bool is_hide() const { return state_ == State::Hide; }
  bool is_none() const { return state_ == State::None; }
  bool is_show() const { return state_ == State::Show; }

 private:
  gfx::RectF bounds_;
  State state_;

  DISALLOW_COPY_AND_ASSIGN(LayoutCaret);
};

}  // namespace views

#endif  // EVITA_LAYOUT_LAYOUT_CARET_H_
