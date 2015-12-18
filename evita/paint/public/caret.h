// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_PAINT_PUBLIC_CARET_H_
#define EVITA_PAINT_PUBLIC_CARET_H_

#include "base/macros.h"
#include "evita/gfx/rect_f.h"

namespace paint {

enum class CaretState {
  Hide,
  None,
  Show,
};

//////////////////////////////////////////////////////////////////////
//
// Caret
//
class Caret final {
 public:
  Caret(CaretState state, const gfx::RectF& bounds);
  ~Caret();

  const gfx::RectF& bounds() const { return bounds_; }

  bool is_hide() const { return state_ == CaretState::Hide; }
  bool is_none() const { return state_ == CaretState::None; }
  bool is_show() const { return state_ == CaretState::Show; }

 private:
  gfx::RectF bounds_;
  CaretState state_;

  DISALLOW_COPY_AND_ASSIGN(Caret);
};

}  // namespace paint

#endif  // EVITA_PAINT_PUBLIC_CARET_H_
