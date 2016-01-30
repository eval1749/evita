// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_CONTROLS_BUTTON_PAINTER_H_
#define EVITA_UI_CONTROLS_BUTTON_PAINTER_H_

#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "evita/gfx/rect_f.h"
#include "evita/ui/controls/control.h"

namespace gfx {
class Canvas;
}

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// ButtonPainter
//
class ButtonPainter final {
 public:
  ButtonPainter();
  ~ButtonPainter();

  void Paint(gfx::Canvas* canvas,
             const gfx::RectF& bounds,
             Control::State state,
             const Control::Style& style,
             base::StringPiece16 text) const;

 private:
  DISALLOW_COPY_AND_ASSIGN(ButtonPainter);
};

}  // namespace ui

#endif  // EVITA_UI_CONTROLS_BUTTON_PAINTER_H_
