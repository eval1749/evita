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
#include "evita/visuals/model/box_visitor.h"
#include "evita/visuals/model/line_box.h"
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
    PaintVisitor* const painter_;

    DISALLOW_COPY_AND_ASSIGN(BoxPaintScope);
  };

#define V(name) void Visit##name(name* box) final;
  FOR_EACH_VISUAL_BOX(V)
#undef V

  bool NeedsPaintContainerBox(const ContainerBox& box) const;
  bool NeedsPaintInlineBox(const InlineBox& box) const;
  void PaintDecoration(const Box& box);
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

bool PaintVisitor::NeedsPaintContainerBox(const ContainerBox& box) const {
  if (box.is_display_none() || box.bounds().size().IsEmpty())
    return false;
  if (!box.IsContentDirty() && !box.IsChildContentDirty())
    return false;
  const auto& bounds = transformer_.MapRect(box.bounds());
  return paint_info_.cull_rect().Intersects(bounds);
}

bool PaintVisitor::NeedsPaintInlineBox(const InlineBox& box) const {
  if (box.is_display_none() || box.bounds().size().IsEmpty())
    return false;
  if (!box.IsContentDirty())
    return false;
  const auto& bounds = transformer_.MapRect(box.bounds());
  return paint_info_.cull_rect().Intersects(bounds);
}

std::unique_ptr<DisplayItemList> PaintVisitor::Paint(const Box& box) {
  DCHECK(box.IsLayoutClean());
  Visit(box);
  return builder_.Build();
}

void PaintVisitor::PaintDecoration(const Box& box) {
  if (box.background().color() != css::Color()) {
    builder_.AddNew<FillRectDisplayItem>(FloatRect(box.bounds().size()),
                                         box.background().color().value());
  }
  const auto& border = box.border();
  if (!border.HasValue())
    return;

#if USE_SIMPLE_BORDER
  if (IsSimpleBorder(border) && border.top_color() != FloatColor()) {
    builder_.AddNew<DrawRectDisplayItem>(FloatRect(box.bounds().size()),
                                         border.top_color(), border.top());
    return;
  }
#endif

  if (border.top() && border.top_color() != css::Color()) {
    builder_.AddNew<FillRectDisplayItem>(
        FloatRect(FloatPoint(), FloatSize(box.bounds().width(), border.top())),
        border.top_color().value());
  }
  if (border.left() && border.left_color() != css::Color()) {
    builder_.AddNew<FillRectDisplayItem>(
        FloatRect(FloatPoint(),
                  FloatSize(border.left(), box.bounds().height())),
        border.left_color().value());
  }
  if (border.right() && border.right_color() != css::Color()) {
    builder_.AddNew<FillRectDisplayItem>(
        FloatRect(FloatPoint(box.bounds().width() - border.right(), 0),
                  FloatSize(border.right(), box.bounds().height())),
        border.right_color().value());
  }
  if (border.bottom() && border.bottom_color() != css::Color()) {
    builder_.AddNew<FillRectDisplayItem>(
        FloatRect(FloatPoint(0, box.bounds().height() - border.bottom()),
                  FloatSize(box.bounds().width(), border.bottom())),
        border.bottom_color().value());
  }
}

void PaintVisitor::PopTransform() {
  const auto& last_transforms = transforms_.top();
  transforms_.pop();
  if (last_transforms == transforms_.top())
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

void PaintVisitor::VisitTextBox(TextBox* text) {
  if (!NeedsPaintInlineBox(*text))
    return;
  BoxPaintScope paint_scope(this, *text);
  builder_.AddNew<DrawTextDisplayItem>(text->content_bounds(), text->color(),
                                       text->baseline(), text->text());
}

//////////////////////////////////////////////////////////////////////
//
// PaintVisitor::BoxPaintScope
//
PaintVisitor::BoxPaintScope::BoxPaintScope(PaintVisitor* painter,
                                           const Box& box)
    : painter_(painter) {
  painter_->builder_.AddRect(painter_->transformer_.MapRect(box.bounds()));
  painter_->transformer_.Translate(box.bounds().origin());
  painter_->PushTransform();
  painter_->PaintDecoration(box);
  const auto& content_bounds = box.content_bounds();
  painter_->transformer_.Translate(content_bounds.origin());
  painter_->PushTransform();
  painter_->builder_.AddNew<BeginClipDisplayItem>(
      FloatRect(content_bounds.size()));
}

PaintVisitor::BoxPaintScope::~BoxPaintScope() {
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

std::unique_ptr<DisplayItemList> Painter::Paint(const PaintInfo& paint_info,
                                                const Box& box) {
  return PaintVisitor(paint_info).Paint(box);
}

}  // namespace visuals
