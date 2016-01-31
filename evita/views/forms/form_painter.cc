// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/forms/form_painter.h"

#include "evita/dom/public/caret_shape.h"
#include "evita/dom/public/form.h"
#include "evita/dom/public/form_controls.h"
#include "evita/dom/public/form_control_visitor.h"
#include "evita/gfx/canvas.h"
#include "evita/gfx/color_f.h"
#include "evita/ui/controls/button_painter.h"
#include "evita/ui/controls/checkbox_painter.h"
#include "evita/ui/controls/control.h"
#include "evita/ui/controls/label_painter.h"
#include "evita/ui/controls/radio_button_painter.h"
#include "evita/ui/controls/text_field_painter.h"
#include "evita/ui/system_metrics.h"
#include "evita/views/forms/form_paint_info.h"
#include "evita/views/forms/form_paint_state.h"

namespace views {

using FormControl = domapi::FormControl;
using FormControlVisitor = domapi::FormControlVisitor;
using IntRect = domapi::IntRect;
using IntSize = domapi::IntSize;

namespace {

ui::Control::Style ComputeControlStyle() {
  ui::Control::Style style;
  style.bgcolor = ui::SystemMetrics::instance()->bgcolor();
  style.color = ui::SystemMetrics::instance()->color();
  style.font_family = ui::SystemMetrics::instance()->font_family();
  style.font_size = ui::SystemMetrics::instance()->font_size();
  style.gray_text = ui::SystemMetrics::instance()->gray_text();
  style.highlight = ui::SystemMetrics::instance()->highlight();
  style.hotlight = ui::SystemMetrics::instance()->hotlight();
  style.shadow = ui::SystemMetrics::instance()->shadow();
  return style;
}

ui::Control::State ComputeControlState(const FormControl::State& state) {
  if (state.disabled())
    return ui::Control::State::Disabled;
  if (state.focused())
    return ui::Control::State::Highlight;
  if (state.hovered())
    return ui::Control::State::Hovered;
  return ui::Control::State::Normal;
}

gfx::RectF ToRectF(const IntRect& rect) {
  return gfx::RectF(rect.x(), rect.y(), rect.right(), rect.bottom());
}

gfx::RectF ToRectF(const IntSize& size) {
  return gfx::RectF(gfx::PointF(), gfx::SizeF(size.width(), size.height()));
}

//////////////////////////////////////////////////////////////////////
//
// PaintVisitor
//
class PaintVisitor : public FormControlVisitor {
 public:
  PaintVisitor(const FormPaintInfo& paint_info,
               const ui::Control::Style& style);
  ~PaintVisitor() final = default;

 private:
// FormControlVisitor
#define V(Name) void Visit##Name(domapi::Name* control) final;
  FOR_EACH_DOMAPI_FORM_CONTROL(V)
#undef V

  const FormPaintInfo& paint_info_;
  const ui::Control::Style& style_;

  DISALLOW_COPY_AND_ASSIGN(PaintVisitor);
};

PaintVisitor::PaintVisitor(const FormPaintInfo& paint_info,
                           const ui::Control::Style& style)
    : paint_info_(paint_info), style_(style) {}

void PaintVisitor::VisitButton(domapi::Button* button) {
  ui::ButtonPainter().Paint(
      paint_info_.canvas(), ToRectF(button->bounds().size()),
      ComputeControlState(button->state()), style_, button->label());
}

void PaintVisitor::VisitCheckbox(domapi::Checkbox* checkbox) {
  ui::CheckboxPainter().Paint(
      paint_info_.canvas(), ToRectF(checkbox->bounds().size()),
      ComputeControlState(checkbox->state()), style_, checkbox->checked());
}

void PaintVisitor::VisitLabel(domapi::Label* label) {
  ui::LabelPainter().Paint(
      paint_info_.canvas(), ToRectF(label->bounds().size()),
      ComputeControlState(label->state()), style_, label->text());
}

void PaintVisitor::VisitTextField(domapi::TextField* text_field) {
  static_assert(static_cast<int>(ui::TextFieldControl::CaretShape::Bar) ==
                    static_cast<int>(domapi::CaretShape::Bar),
                "CaretShape::Bar mismatched");
  static_assert(static_cast<int>(ui::TextFieldControl::CaretShape::Box) ==
                    static_cast<int>(domapi::CaretShape::Box),
                "CaretShape::Box mismatched");
  static_assert(static_cast<int>(ui::TextFieldControl::CaretShape::None) ==
                    static_cast<int>(domapi::CaretShape::None),
                "CaretShape::None mismatched");
  ui::TextFieldControl::Selection selection;
  selection.caret_shape = static_cast<ui::TextFieldControl::CaretShape>(
      text_field->selection().caret_shape());
  selection.anchor_offset = text_field->selection().anchor_offset();
  selection.focus_offset = text_field->selection().focus_offset();
  ui::TextFieldPainter().Paint(
      paint_info_.canvas(), ToRectF(text_field->bounds().size()),
      ComputeControlState(text_field->state()), style_, selection,
      text_field->scroll_left(), text_field->text());
}

void PaintVisitor::VisitRadioButton(domapi::RadioButton* radio_button) {
  ui::RadioButtonPainter().Paint(paint_info_.canvas(),
                                 ToRectF(radio_button->bounds().size()),
                                 ComputeControlState(radio_button->state()),
                                 style_, radio_button->checked());
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// FormPainter
//
FormPainter::FormPainter() {}
FormPainter::~FormPainter() {}

void FormPainter::Paint(const FormPaintInfo& paint_info,
                        const domapi::Form& form) {
  const auto& style = ComputeControlStyle();
  const auto& canvas = paint_info.canvas();
  gfx::Canvas::DrawingScope drawing_scope(canvas);
  if (canvas->should_clear()) {
    canvas->AddDirtyRect(paint_info.bounds());
    canvas->Clear(style.bgcolor);
  }
  PaintVisitor painter(paint_info, style);
  for (const auto& control : form.controls()) {
    gfx::Canvas::ScopedState scope(paint_info.canvas());
    paint_info.canvas()->SetOffsetBounds(ToRectF(control->bounds()));
    painter.Visit(*control);
  }
}

}  // namespace views
