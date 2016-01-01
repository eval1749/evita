// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define PRINT_DIRTY 0
#define PAINT_DIRTY 0

#include <dwrite.h>

#if PRINT_DIRTY
#include <iostream>
#endif
#include <stack>

#include "evita/visuals/display/display_item_list_processor.h"

#include "evita/gfx/brush.h"
#include "evita/gfx/canvas.h"
#include "evita/gfx/color_f.h"
#include "evita/gfx/text_format.h"
#include "evita/visuals/display/public/display_items.h"
#include "evita/visuals/display/public/display_item_list.h"
#include "evita/visuals/display/public/display_item_visitor.h"
#include "evita/visuals/geometry/float_rect.h"

namespace visuals {

namespace {

gfx::ColorF ToColorF(const FloatColor& color) {
  return gfx::ColorF(color.red(), color.green(), color.blue(), color.alpha());
}

D2D1_MATRIX_3X2_F ToMatrix3x2F(const FloatMatrix3x2& matrix) {
  return D2D1_MATRIX_3X2_F{matrix.data()[0], matrix.data()[1],
                           matrix.data()[2], matrix.data()[3],
                           matrix.data()[4], matrix.data()[5]};
}

gfx::PointF ToPointF(const FloatPoint& point) {
  return gfx::PointF(point.x(), point.y());
}

gfx::SizeF ToSizeF(const FloatSize& size) {
  return gfx::SizeF(size.width(), size.height());
}

gfx::RectF ToRectF(const FloatRect& rect) {
  return gfx::RectF(ToPointF(rect.origin()), ToSizeF(rect.size()));
}

//////////////////////////////////////////////////////////////////////
//
// PaintVisitor
//
class PaintVisitor final : public DisplayItemVisitor {
 public:
  explicit PaintVisitor(gfx::Canvas* canvas) : canvas_(canvas) {}
  ~PaintVisitor() final = default;

 private:
// DisplayItemPaintVisitor
#define V(name) void Visit##name(name##DisplayItem* item) final;
  FOR_EACH_DISPLAY_ITEM(V)
#undef V

  gfx::Canvas* const canvas_;
  std::stack<D2D1_MATRIX_3X2_F> transforms_;

  DISALLOW_COPY_AND_ASSIGN(PaintVisitor);
};

void PaintVisitor::VisitBeginClip(BeginClipDisplayItem* item) {
  (*canvas_)->PushAxisAlignedClip(ToRectF(item->bounds()),
                                  D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
}

void PaintVisitor::VisitBeginTransform(BeginTransformDisplayItem* item) {
  D2D1_MATRIX_3X2_F current_transform;
  (*canvas_)->GetTransform(&current_transform);
  transforms_.push(current_transform);
  (*canvas_)->SetTransform(ToMatrix3x2F(item->matrix()));
}

void PaintVisitor::VisitDrawRect(DrawRectDisplayItem* item) {
  canvas_->DrawRectangle(gfx::Brush(canvas_, ToColorF(item->color())),
                         ToRectF(item->bounds()), item->thickness());
}

void PaintVisitor::VisitDrawText(DrawTextDisplayItem* item) {
  gfx::TextFormat text_format(L"Arial", 13);
  canvas_->DrawText(text_format, gfx::Brush(canvas_, ToColorF(item->color())),
                    ToRectF(item->bounds()), item->text());
}

void PaintVisitor::VisitEndClip(EndClipDisplayItem* item) {
  (*canvas_)->PopAxisAlignedClip();
}

void PaintVisitor::VisitEndTransform(EndTransformDisplayItem* item) {
  (*canvas_)->SetTransform(transforms_.top());
  transforms_.pop();
}

void PaintVisitor::VisitFillRect(FillRectDisplayItem* item) {
  canvas_->FillRectangle(gfx::Brush(canvas_, ToColorF(item->color())),
                         ToRectF(item->bounds()));
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// DisplayItemListProcessor
//
DisplayItemListProcessor::DisplayItemListProcessor() {}
DisplayItemListProcessor::~DisplayItemListProcessor() {}

void DisplayItemListProcessor::Paint(gfx::Canvas* canvas,
                                     std::unique_ptr<DisplayItemList> list) {
  gfx::Canvas::DrawingScope drawing_scope(canvas);
  for (const auto& rect : list->rects())
    canvas->AddDirtyRect(ToRectF(rect));
  PaintVisitor painter(canvas);
  for (const auto& item : list->items())
    painter.Visit(item);

#if PRINT_DIRTY
  std::cout << "DisplayItemListProcessor::Paint(): dirty rects" << std::endl;
  auto index = 0;
  for (const auto& rect : list->rects())
    std::cout << ' ' << ++index << ' ' << rect;
  std::cout << std::endl;
#endif
#if PAINT_DIRTY
  for (const auto& rect : list->rects()) {
    gfx::Brush brush(canvas, gfx::ColorF(1, 0, 0, 0.1f));
    const auto& rect_f = ToRectF(rect);
    canvas->FillRectangle(brush, rect_f);
    canvas->DrawRectangle(brush, rect_f);
  }
#endif
}

}  // namespace visuals
