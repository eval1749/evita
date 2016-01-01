// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stack>

#include "evita/visuals/paint/painter.h"

#include "base/logging.h"
#include "evita/visuals/display/display_item_list_builder.h"
#include "evita/visuals/display/public/display_items.h"
#include "evita/visuals/display/public/display_item_list.h"
#include "evita/visuals/geometry/affine_transformer.h"
#include "evita/visuals/geometry/float_rect.h"
#include "evita/visuals/model/block_box.h"
#include "evita/visuals/model/box_editor.h"
#include "evita/visuals/model/box_traversal.h"
#include "evita/visuals/model/box_visitor.h"
#include "evita/visuals/model/line_box.h"
#include "evita/visuals/model/root_box.h"
#include "evita/visuals/model/text_box.h"
#include "evita/visuals/paint/paint_info.h"

// TODO(eval1749): Drawing rectangle with thickness doesn't work as expected.
// I get 1 pixel vertical line for 4 pixel border.
#define USE_SIMPLE_BORDER 0

namespace visuals {

namespace {

#if USE_SIMPLE_BORDER
bool IsSimpleBorder(const Border& border) {
  return border.top() == border.bottom() && border.top() == border.left() &&
         border.top() == border.right() &&
         border.top_color() == border.bottom_color() &&
         border.top_color() == border.left_color() &&
         border.top_color() == border.right_color();
}
#endif

bool IsBackgroundChanged(const Box& box) {
  if (!box.background().HasValue())
    return false;
  return box.IsBackgroundChanged() || box.IsOriginChanged() ||
         box.IsSizeChanged() || box.IsPaddingChanged();
}

bool IsBorderChanged(const Box& box) {
  if (!box.border().HasValue())
    return false;
  return box.IsBorderChanged() || box.IsOriginChanged() || box.IsSizeChanged();
}

//////////////////////////////////////////////////////////////////////
//
// PaintVisitor
//
class PaintVisitor final : public BoxVisitor {
 public:
  explicit PaintVisitor(const PaintInfo& paint_info);
  ~PaintVisitor() final;

  std::unique_ptr<DisplayItemList> Paint(const Box& box);

 private:
  class BoxPaintScope final {
   public:
    BoxPaintScope(PaintVisitor* painter, const Box& box);
    ~BoxPaintScope();

   private:
    const Box& box_;
    PaintVisitor* const painter_;

    DISALLOW_COPY_AND_ASSIGN(BoxPaintScope);
  };

#define V(name) void Visit##name(name* box) final;
  FOR_EACH_VISUAL_BOX(V)
#undef V

  void AddDirtyBounds(const FloatRect& bounds);
  void FillRect(const FloatRect& rect, const FloatColor& color);
  void FillRectAndMark(const FloatRect& rect,
                       const FloatColor& color,
                       bool mark);
  bool NeedsPaintContainerBox(const ContainerBox& box) const;
  bool NeedsPaintInlineBox(const InlineBox& box) const;
  void PaintBackgroundINeeded(const Box& box);
  void PaintBorderINeeded(const Box& box);
  void PopTransform();
  void PushTransform();

  DisplayItemListBuilder builder_;
  const PaintInfo& paint_info_;
  AffineTransformer transformer_;
  std::stack<FloatMatrix3x2> transforms_;

  DISALLOW_COPY_AND_ASSIGN(PaintVisitor);
};

PaintVisitor::PaintVisitor(const PaintInfo& paint_info)
    : builder_(paint_info.cull_rect()), paint_info_(paint_info) {
  transforms_.push(transformer_.matrix());
}

PaintVisitor::~PaintVisitor() {
  transforms_.pop();
  DCHECK(transforms_.empty());
}

void PaintVisitor::AddDirtyBounds(const FloatRect& bounds) {
  builder_.AddRect(transformer_.MapRect(bounds));
}

void PaintVisitor::FillRect(const FloatRect& rect, const FloatColor& color) {
  DCHECK(!rect.IsEmpty());
  if (color.alpha() == 0)
    return;
  builder_.AddNew<FillRectDisplayItem>(rect, color);
}

void PaintVisitor::FillRectAndMark(const FloatRect& rect,
                                   const FloatColor& color,
                                   bool mark) {
  DCHECK(!rect.IsEmpty());
  if (color.alpha() == 0)
    return;
  if (mark)
    AddDirtyBounds(rect);
  builder_.AddNew<FillRectDisplayItem>(rect, color);
}

bool PaintVisitor::NeedsPaintContainerBox(const ContainerBox& box) const {
  if (box.is_display_none() || box.bounds().size().IsEmpty())
    return false;
#if 0
  if (!box.ShouldPaint() && !box.IsBackgroundChanged() &&
      !box.IsBorderChanged())
    return false;
#endif
  const auto& bounds = transformer_.MapRect(box.bounds());
  return paint_info_.cull_rect().Intersects(bounds);
}

bool PaintVisitor::NeedsPaintInlineBox(const InlineBox& box) const {
  if (box.is_display_none() || box.bounds().size().IsEmpty())
    return false;
#if 0
  if (!box.ShouldPaint() && !box.IsBackgroundChanged() &&
      !box.IsBorderChanged() && !box.IsContentChanged()) {
    return false;
  }
#endif
  const auto& bounds = transformer_.MapRect(box.bounds());
  return paint_info_.cull_rect().Intersects(bounds);
}

std::unique_ptr<DisplayItemList> PaintVisitor::Paint(const Box& box) {
  Visit(box);
  return builder_.Build();
}

void PaintVisitor::PaintBackgroundINeeded(const Box& box) {
  const auto is_background_changed = IsBackgroundChanged(box);
#if 0
  if (!box.ShouldPaint() && !is_background_changed)
    return;
#endif
  FillRectAndMark(FloatRect(box.bounds().size()),
                  box.background().color().value(), is_background_changed);
}

void PaintVisitor::PaintBorderINeeded(const Box& box) {
  const auto is_border_changed = IsBorderChanged(box);
#if 0
  if (!box.ShouldPaint() && !is_border_changed)
    return;
#endif
  const auto& border = box.border();
  if (border.top()) {
    FillRectAndMark(
        FloatRect(FloatPoint(), FloatSize(box.bounds().width(), border.top())),
        border.top_color().value(), is_border_changed);
  }
  if (border.left()) {
    FillRectAndMark(FloatRect(FloatPoint(),
                              FloatSize(border.left(), box.bounds().height())),
                    border.left_color().value(), is_border_changed);
  }
  if (border.right()) {
    FillRectAndMark(
        FloatRect(FloatPoint(box.bounds().width() - border.right(), 0),
                  FloatSize(border.right(), box.bounds().height())),
        border.right_color().value(), is_border_changed);
  }
  if (border.bottom()) {
    FillRectAndMark(
        FloatRect(FloatPoint(0, box.bounds().height() - border.bottom()),
                  FloatSize(box.bounds().width(), border.bottom())),
        border.bottom_color().value(), is_border_changed);
  }
}

void PaintVisitor::PopTransform() {
  const auto& last_transform = transforms_.top();
  transforms_.pop();
  if (last_transform == transforms_.top())
    return;
  transformer_.set_matrix(transforms_.top());
  builder_.AddNew<EndTransformDisplayItem>();
}

void PaintVisitor::PushTransform() {
  const auto& transform = transformer_.matrix();
  if (transform != transforms_.top())
    builder_.AddNew<BeginTransformDisplayItem>(transform);
  transforms_.push(transform);
}

// BoxVisitor
void PaintVisitor::VisitBlockBox(BlockBox* block) {
  if (!NeedsPaintContainerBox(*block))
    return;
  BoxPaintScope paint_scope(this, *block);
  for (const auto& child : block->child_boxes())
    Visit(child);
}

void PaintVisitor::VisitLineBox(LineBox* line) {
  if (!NeedsPaintContainerBox(*line))
    return;
  BoxPaintScope paint_scope(this, *line);
  for (const auto& child : line->child_boxes())
    Visit(child);
}

void PaintVisitor::VisitRootBox(RootBox* root) {
  Visit(root->first_child());
}

void PaintVisitor::VisitTextBox(TextBox* text) {
  const auto& bounds = text->content_bounds();
  if (text->IsContentChanged() || text->IsOriginChanged())
    AddDirtyBounds(bounds);
  BoxPaintScope paint_scope(this, *text);
  if (text->background().HasValue()) {
    // TODO(eval1749): We should have a member function for painting background
    // with clip bounds.
    FillRect(bounds, text->background().color().value());
  }
  builder_.AddNew<DrawTextDisplayItem>(bounds, text->color(), text->baseline(),
                                       text->text());
}

//////////////////////////////////////////////////////////////////////
//
// PaintVisitor::BoxPaintScope
//
PaintVisitor::BoxPaintScope::BoxPaintScope(PaintVisitor* painter,
                                           const Box& box)
    : box_(box), painter_(painter) {
  painter_->transformer_.Translate(box.bounds().origin());
  painter_->PushTransform();
  painter_->PaintBackgroundINeeded(box);
  painter_->PaintBorderINeeded(box);
  const auto& content_bounds = box.content_bounds();
  painter_->transformer_.Translate(content_bounds.origin());
  painter_->PushTransform();
  painter_->builder_.AddNew<BeginClipDisplayItem>(
      FloatRect(content_bounds.size()));
}

PaintVisitor::BoxPaintScope::~BoxPaintScope() {
  BoxEditor().DidPaint(&const_cast<Box&>(box_));
  painter_->builder_.AddNew<EndClipDisplayItem>();
  painter_->PopTransform();
  painter_->PopTransform();
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Painter
//
Painter::Painter() {}
Painter::~Painter() {}

// TODO(eval1749): |Painter::Paint()| should be called after layout. We should
// make sure to do so.
std::unique_ptr<DisplayItemList> Painter::Paint(const PaintInfo& paint_info,
                                                const Box& box) {
  return PaintVisitor(paint_info).Paint(box);
}

}  // namespace visuals
