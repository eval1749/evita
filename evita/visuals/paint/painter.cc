// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <stack>

#include "evita/visuals/paint/painter.h"

#include "base/logging.h"
#include "base/trace_event/trace_event.h"
#include "evita/gfx/base/geometry/affine_transformer.h"
#include "evita/gfx/base/geometry/float_rect.h"
#include "evita/visuals/display/display_item_list_builder.h"
#include "evita/visuals/display/public/display_item_list.h"
#include "evita/visuals/display/public/display_items.h"
#include "evita/visuals/fonts/font_description_builder.h"
#include "evita/visuals/fonts/text_format_factory.h"
#include "evita/visuals/fonts/text_layout.h"
#include "evita/visuals/layout/box_editor.h"
#include "evita/visuals/layout/box_selection.h"
#include "evita/visuals/layout/box_traversal.h"
#include "evita/visuals/layout/box_visitor.h"
#include "evita/visuals/layout/flow_box.h"
#include "evita/visuals/layout/image_box.h"
#include "evita/visuals/layout/root_box.h"
#include "evita/visuals/layout/shape_box.h"
#include "evita/visuals/layout/text_box.h"
#include "evita/visuals/paint/paint_info.h"
#include "evita/visuals/view/public/selection.h"
#include "evita/visuals/view/public/view_lifecycle.h"

// TODO(eval1749): Drawing rectangle with thickness doesn't work as expected.
// I get 1 pixel vertical line for 4 pixel border.
#define USE_SIMPLE_BORDER 0

namespace visuals {

namespace {

#if USE_SIMPLE_BORDER
#endif

bool IsBackgroundChanged(const Box& box) {
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

  // The entry point of |PaintVisitor|.
  std::unique_ptr<DisplayItemList> Paint(const RootBox& root_box);

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

  void AddDirtyBounds(const gfx::FloatRect& bounds);
  void FillRect(const gfx::FloatRect& rect, const gfx::FloatColor& color);
  void FillRectAndMark(const gfx::FloatRect& rect,
                       const gfx::FloatColor& color,
                       bool mark);
  bool NeedsPaintContainerBox(const ContainerBox& box) const;
  bool NeedsPaintContentBox(const ContentBox& box) const;
  void PaintBackgroundINeeded(const Box& box);
  void PaintBorderINeeded(const Box& box);
  void PaintContainerBox(const ContainerBox& box);
  void PaintSelectionIfNeeded(const Box& box);
  void PopTransform();
  void PushTransform();

  DisplayItemListBuilder builder_;
  const PaintInfo& paint_info_;
  gfx::AffineTransformer transformer_;
  std::stack<gfx::FloatMatrix3x2> transforms_;

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

void PaintVisitor::AddDirtyBounds(const gfx::FloatRect& bounds) {
  builder_.AddRect(transformer_.MapRect(bounds));
}

void PaintVisitor::FillRect(const gfx::FloatRect& rect,
                            const gfx::FloatColor& color) {
  DCHECK(!rect.IsEmpty());
  if (color.alpha() == 0)
    return;
  builder_.AddNew<FillRectDisplayItem>(rect, color);
}

void PaintVisitor::FillRectAndMark(const gfx::FloatRect& rect,
                                   const gfx::FloatColor& color,
                                   bool mark) {
  DCHECK(!rect.IsEmpty());
  if (mark)
    AddDirtyBounds(rect);
  if (color.alpha() == 0)
    return;
  builder_.AddNew<FillRectDisplayItem>(rect, color);
}

bool PaintVisitor::NeedsPaintContainerBox(const ContainerBox& box) const {
  if (box.bounds().size().IsEmpty())
    return false;
#if 0
  if (!box.ShouldPaint() && !box.IsBackgroundChanged() &&
      !box.IsBorderChanged())
    return false;
#endif
  const auto& bounds = transformer_.MapRect(box.bounds());
  return paint_info_.cull_rect().Intersects(bounds);
}

bool PaintVisitor::NeedsPaintContentBox(const ContentBox& box) const {
  if (box.bounds().size().IsEmpty())
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

// The entry point of |PaintVisitor|.
std::unique_ptr<DisplayItemList> PaintVisitor::Paint(const RootBox& root_box) {
  // TODO(eval1749): Should we need to pass dirty rect list?
  AddDirtyBounds(root_box.bounds());
  Visit(root_box);
  if (!paint_info_.debug_text().empty()) {
    const auto& font =
        FontDescription::Builder().SetFamily(L"Arial").SetSize(10).Build();
    const auto& text_format = TextFormatFactory::GetInstance()->Get(font);
    const auto& debug_text_bounds =
        gfx::FloatRect(root_box.bounds().top_right() - gfx::FloatSize(200, 0),
                       gfx::FloatSize(200, 50));
    const auto& text_layout = TextLayout(text_format, paint_info_.debug_text(),
                                         debug_text_bounds.size());
    builder_.AddNew<DrawTextDisplayItem>(debug_text_bounds,
                                         gfx::FloatColor(1, 0, 0), 20,
                                         text_layout, paint_info_.debug_text());
  }
  return builder_.Build();
}

void PaintVisitor::PaintBackgroundINeeded(const Box& box) {
  if (box.bounds().size().IsEmpty())
    return;
  const auto is_background_changed = IsBackgroundChanged(box);
#if 0
  if (!box.ShouldPaint() && !is_background_changed)
    return;
#endif
  FillRectAndMark(gfx::FloatRect(box.bounds().size()), box.background_color(),
                  is_background_changed);
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
        gfx::FloatRect(gfx::FloatPoint(),
                       gfx::FloatSize(box.bounds().width(), border.top())),
        border.top_color(), is_border_changed);
  }
  if (border.left()) {
    FillRectAndMark(
        gfx::FloatRect(gfx::FloatPoint(),
                       gfx::FloatSize(border.left(), box.bounds().height())),
        border.left_color(), is_border_changed);
  }
  if (border.right()) {
    FillRectAndMark(
        gfx::FloatRect(
            gfx::FloatPoint(box.bounds().width() - border.right(), 0),
            gfx::FloatSize(border.right(), box.bounds().height())),
        border.right_color(), is_border_changed);
  }
  if (border.bottom()) {
    FillRectAndMark(
        gfx::FloatRect(
            gfx::FloatPoint(0, box.bounds().height() - border.bottom()),
            gfx::FloatSize(box.bounds().width(), border.bottom())),
        border.bottom_color(), is_border_changed);
  }
}

void PaintVisitor::PaintContainerBox(const ContainerBox& box) {
  if (!NeedsPaintContainerBox(box))
    return;
  BoxPaintScope paint_scope(this, box);
  for (const auto& child : box.child_boxes()) {
    Visit(child);
    PaintSelectionIfNeeded(*child);
  }
}

// Note: We Since caret can be placed after text, we should paint caret and
// selection outside text box clip area.
void PaintVisitor::PaintSelectionIfNeeded(const Box& box) {
  const auto text = box.as<TextBox>();
  if (!text)
    return;
  const auto& selection = text->root_box()->selection();
  if (selection.is_none())
    return;
  if (selection.focus_box() != text)
    return;
  const auto& focus_box_rect =
      text->text_layout().HitTestTextPosition(selection.focus_offset());
  if (selection.is_range()) {
    DCHECK_EQ(selection.anchor_box(), selection.focus_box());
    const auto& anchor_box_rect =
        text->text_layout().HitTestTextPosition(selection.anchor_offset());
    const auto is_anchor_start = anchor_box_rect.x() < focus_box_rect.x();
    const auto& selection_rect = gfx::FloatRect(
        is_anchor_start ? anchor_box_rect.origin() : focus_box_rect.origin(),
        gfx::FloatSize(
            is_anchor_start ? focus_box_rect.x() - anchor_box_rect.x()
                            : anchor_box_rect.x() - focus_box_rect.x(),
            std::max(anchor_box_rect.height(), focus_box_rect.height())));
    FillRect(selection_rect, selection.selection_color());
  }
  if (selection.caret_shape().is_none())
    return;
  FillRect(gfx::FloatRect(focus_box_rect.origin(),
                          gfx::FloatSize(1, focus_box_rect.height())),
           selection.caret_color());
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
void PaintVisitor::VisitFlowBox(FlowBox* box) {
  PaintContainerBox(*box);
}

void PaintVisitor::VisitImageBox(ImageBox* image) {
  const auto& data = image->data();
  if (data.bounds().IsEmpty()) {
    BoxEditor().DidPaint(image);
    return;
  }
  const auto& bounds = gfx::FloatRect(image->content_bounds().size());
  if (image->IsContentChanged() || image->IsOriginChanged())
    AddDirtyBounds(bounds);
  BoxPaintScope paint_scope(this, *image);
  builder_.AddNew<DrawBitmapDisplayItem>(
      gfx::FloatRect(image->point(), data.bounds().size()), image->bitmap(),
      data.bounds(), image->opacity());
  BoxEditor().DidPaint(image);
}

void PaintVisitor::VisitRootBox(RootBox* root) {
  // TODO(eval1749): We should not paint background if document element, which
  // size is viewport, has background.
  BoxPaintScope paint_scope(this, *root);
  Visit(root->first_child());
  BoxEditor().DidPaint(root);
}

void PaintVisitor::VisitShapeBox(ShapeBox* shape) {
  const auto& bounds = gfx::FloatRect(shape->content_bounds().size());
  if (shape->IsContentChanged() || shape->IsOriginChanged())
    AddDirtyBounds(bounds);
  BoxPaintScope paint_scope(this, *shape);
  builder_.AddNew<DrawRectDisplayItem>(bounds, shape->color(), 1);
  BoxEditor().DidPaint(shape);
}

void PaintVisitor::VisitTextBox(TextBox* text) {
  const auto& bounds = gfx::FloatRect(text->content_bounds().size());
  if (bounds.size().IsEmpty())
    return;
  if (text->IsContentChanged() || text->IsOriginChanged())
    AddDirtyBounds(bounds);
  BoxPaintScope paint_scope(this, *text);
  if (text->background_color() != gfx::FloatColor()) {
    // TODO(eval1749): We should have a member function for painting background
    // with clip bounds.
    FillRect(bounds, text->background_color());
  }
  if (!text->has_text_layout())
    return;
  builder_.AddNew<DrawTextDisplayItem>(bounds, text->color(), text->baseline(),
                                       text->text_layout(), text->data());
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
      gfx::FloatRect(content_bounds.size()));
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

std::unique_ptr<DisplayItemList> Painter::Paint(const PaintInfo& paint_info,
                                                const RootBox& root_box) {
  TRACE_EVENT0("visuals", "Painter::Paint");
  ViewLifecycle::Scope scope(root_box.lifecycle(),
                             ViewLifecycle::State::InPaint);
  return PaintVisitor(paint_info).Paint(root_box);
}

}  // namespace visuals
