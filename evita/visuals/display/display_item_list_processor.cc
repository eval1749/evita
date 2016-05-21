// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define PRINT_DIRTY 0
#define PAINT_DIRTY 0

#include <dwrite.h>
#include <stdint.h>

#if PRINT_DIRTY
#include <iostream>
#endif
#include <stack>

#include "evita/visuals/display/display_item_list_processor.h"

#include "build/build_config.h"
#include "evita/gfx/base/geometry/float_rect.h"
#include "evita/gfx/bitmap.h"
#include "evita/gfx/brush.h"
#include "evita/gfx/canvas.h"
#include "evita/gfx/color_f.h"
#include "evita/gfx/stroke_style.h"
#include "evita/gfx/text_format.h"
#include "evita/visuals/display/public/display_item_list.h"
#include "evita/visuals/display/public/display_item_visitor.h"
#include "evita/visuals/display/public/display_items.h"
#include "evita/visuals/fonts/native_text_layout_win.h"
#include "evita/visuals/fonts/text_layout.h"

#if OS_WIN
#include "evita/gfx/imaging_factory_win.h"
#include "evita/visuals/imaging/native_image_bitmap_win.h"
#else
#error "Unsupported target"
#endif

namespace visuals {

namespace {

gfx::ColorF ToColorF(const gfx::FloatColor& color) {
  return gfx::ColorF(color.red(), color.green(), color.blue(), color.alpha());
}

D2D1_MATRIX_3X2_F ToMatrix3x2F(const gfx::FloatMatrix3x2& matrix) {
  return D2D1_MATRIX_3X2_F{matrix.data()[0], matrix.data()[1],
                           matrix.data()[2], matrix.data()[3],
                           matrix.data()[4], matrix.data()[5]};
}

gfx::PointF ToPointF(const gfx::FloatPoint& point) {
  return gfx::PointF(point.x(), point.y());
}

gfx::SizeF ToSizeF(const gfx::FloatSize& size) {
  return gfx::SizeF(size.width(), size.height());
}

gfx::RectF ToRectF(const gfx::FloatRect& rect) {
  return gfx::RectF(ToPointF(rect.origin()), ToSizeF(rect.size()));
}

std::unique_ptr<gfx::Bitmap> CreateBitmapFromImage(gfx::Canvas* canvas,
                                                   const ImageBitmap& image) {
  common::ComPtr<IWICFormatConverter> converter;
  COM_VERIFY(gfx::ImagingFactory::GetInstance()->impl()->CreateFormatConverter(
      &converter));
  const auto palette = static_cast<IWICPalette*>(nullptr);
  const auto alpha_threshold = 0.0f;
  COM_VERIFY(converter->Initialize(
      image.impl().get().get(), GUID_WICPixelFormat32bppPBGRA,
      WICBitmapDitherTypeNone, palette, alpha_threshold,
      WICBitmapPaletteTypeMedianCut));
  common::ComPtr<ID2D1Bitmap> bitmap;
  COM_VERIFY((*canvas)->CreateBitmapFromWicBitmap(converter, nullptr, &bitmap));
  return std::make_unique<gfx::Bitmap>(canvas, bitmap);
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

void PaintVisitor::VisitClear(ClearDisplayItem* item) {
  canvas_->Clear(ToColorF(item->color()));
}

void PaintVisitor::VisitDrawBitmap(DrawBitmapDisplayItem* item) {
  const auto& bitmap = CreateBitmapFromImage(canvas_, item->bitmap());
  (*canvas_)->DrawBitmap(*bitmap, ToRectF(item->destination()), item->opacity(),
                         D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
                         ToRectF(item->source()));
}

void PaintVisitor::VisitDrawLine(DrawLineDisplayItem* item) {
  const auto& brush = gfx::Brush(canvas_, ToColorF(item->color()));
  canvas_->DrawLine(brush, ToPointF(item->point1()), ToPointF(item->point2()),
                    item->thickness());
}

void PaintVisitor::VisitDrawRect(DrawRectDisplayItem* item) {
  const auto& brush = gfx::Brush(canvas_, ToColorF(item->color()));
  canvas_->DrawRectangle(brush, ToRectF(item->bounds()), item->thickness());
}

void PaintVisitor::VisitDrawText(DrawTextDisplayItem* item) {
  (*canvas_)->DrawTextLayout(ToPointF(item->bounds().origin()),
                             item->text_layout().impl().get().get(),
                             gfx::Brush(canvas_, ToColorF(item->color())),
                             D2D1_DRAW_TEXT_OPTIONS_CLIP);
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
