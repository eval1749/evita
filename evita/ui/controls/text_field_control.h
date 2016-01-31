// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_CONTROLS_TEXT_FIELD_CONTROL_H_
#define EVITA_UI_CONTROLS_TEXT_FIELD_CONTROL_H_

#include <memory>

#include "evita/ui/controls/control.h"

#include "base/strings/string16.h"
#include "evita/gfx/canvas.h"
#include "evita/gfx/rect_f.h"
#include "evita/ui/caret_owner.h"

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// TextFieldControl
//
class TextFieldControl final : public Control, private CaretOwner {
 public:
  enum class CaretShape {
    None,
    Bar,
    Box,
  };

  struct Selection final {
    size_t anchor_offset = 0;
    size_t focus_offset = 0;
    CaretShape caret_shape = CaretShape::None;

    Selection();
    ~Selection();

    bool operator==(const Selection& other) const;
    bool operator!=(const Selection& other) const;

    bool collapsed() const;
    size_t end() const;
    size_t start() const;
  };

  TextFieldControl(ControlController* controller,
                   const Selection& selection,
                   const base::string16& text,
                   const Style& style);
  ~TextFieldControl() final;

  void set_style(const Style& style);
  void set_selection(const Selection& new_selection);
  void set_text(const base::string16& text);

  int MapPointToOffset(const gfx::PointF& point) const;

 private:
  class View;

  // ui::CaretOwner
  void DidFireCaretTimer() final;

  // ui::Control
  void DidChangeState() final;

  // ui::Widget
  void DidChangeBounds() final;
  HCURSOR GetCursorAt(const gfx::Point& point) const final;
  void OnDraw(gfx::Canvas* canvas) final;

  const std::unique_ptr<View> view_;

  DISALLOW_COPY_AND_ASSIGN(TextFieldControl);
};

}  // namespace ui

#endif  // EVITA_UI_CONTROLS_TEXT_FIELD_CONTROL_H_
