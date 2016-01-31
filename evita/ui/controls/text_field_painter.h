// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_CONTROLS_TEXT_FIELD_PAINTER_H_
#define EVITA_UI_CONTROLS_TEXT_FIELD_PAINTER_H_

#include "base/macros.h"
#include "evita/gfx/rect_f.h"
#include "evita/ui/controls/text_field_control.h"

namespace gfx {
class Canvas;
}

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// TextFieldPainter
//
class TextFieldPainter final {
 public:
  TextFieldPainter();
  ~TextFieldPainter();

  void Paint(gfx::Canvas* canvas,
             const gfx::RectF& bounds,
             Control::State state,
             const Control::Style& style,
             const TextFieldControl::Selection& selection,
             float scroll_left,
             const base::string16& text) const;

 private:
  void PaintBorder(gfx::Canvas* canvas,
                   const gfx::RectF& bounds,
                   Control::State state,
                   const Control::Style& style) const;

  void PaintText(gfx::Canvas* canvas,
                 const gfx::RectF& bounds,
                 Control::State state,
                 const Control::Style& style,
                 const TextFieldControl::Selection& selection,
                 float scroll_left,
                 const base::string16& text) const;

  DISALLOW_COPY_AND_ASSIGN(TextFieldPainter);
};

}  // namespace ui

#endif  // EVITA_UI_CONTROLS_TEXT_FIELD_PAINTER_H_
