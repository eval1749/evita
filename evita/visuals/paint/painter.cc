// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/paint/painter.h"

#include "base/logging.h"
#include "evita/visuals/display/display_item_list_builder.h"
#include "evita/visuals/display/public/display_items.h"
#include "evita/visuals/display/public/display_item_list.h"
#include "evita/visuals/geometry/float_rect.h"
#include "evita/visuals/model/block_box.h"
#include "evita/visuals/model/box_visitor.h"
#include "evita/visuals/model/line_box.h"
#include "evita/visuals/model/text_box.h"

namespace visuals {

namespace {
bool IsSimpleBorder(const Border& border) {
  return border.top() == border.bottom() && border.top() == border.left() &&
         border.top() == border.right() &&
         border.top_color() == border.bottom_color() &&
         border.top_color() == border.left_color() &&
         border.top_color() == border.right_color();
}

//////////////////////////////////////////////////////////////////////
//
// PaintVisitor
//
class PaintVisitor final : public BoxVisitor {
 public:
  PaintVisitor() = default;
  ~PaintVisitor() final = default;

  std::unique_ptr<DisplayItemList> Paint(const Box& box);

 private:
  class ClipScope final {
   public:
    ClipScope(PaintVisitor* painter, const FloatRect& bounds);
    ~ClipScope();

   private:
    DisplayItemListBuilder* const builder_;

    DISALLOW_COPY_AND_ASSIGN(ClipScope);
  };

#define V(name) void Visit##name(name* box) final;
  FOR_EACH_VISUAL_BOX(V)
#undef V
  void PaintDecoration(const Box& box);

  DisplayItemListBuilder builder_;

  DISALLOW_COPY_AND_ASSIGN(PaintVisitor);
};

std::unique_ptr<DisplayItemList> PaintVisitor::Paint(const Box& box) {
  DCHECK(box.IsLayoutClean());
  if (box.IsContentDirty())
    Visit(box);
  return builder_.Build();
}

void PaintVisitor::PaintDecoration(const Box& box) {
  if (box.background().color() != FloatColor()) {
    builder_.AddNew<FillRectDisplayItem>(box.content_bounds(),
                                         box.background().color());
  }
  const auto& border = box.border();
  if (!border.HasValue())
    return;
  if (IsSimpleBorder(border) && border.top_color() != FloatColor()) {
    builder_.AddNew<DrawRectDisplayItem>(
        box.content_bounds() - FloatSize(border.top(), border.top()),
        border.top_color(), border.top());
    return;
  }
  if (border.top() && border.top_color() != FloatColor()) {
    builder_.AddNew<FillRectDisplayItem>(
        FloatRect(box.content_bounds().origin() - FloatSize(0, border.top()),
                  FloatSize(box.content_bounds().size().width(), border.top())),
        border.top_color());
  }
}

// BoxVisitor
void PaintVisitor::VisitBlockBox(BlockBox* block) {
  PaintDecoration(*block);
  ClipScope clip_scope(this, block->content_bounds());
  for (const auto& child : block->child_boxes())
    Visit(child);
}

void PaintVisitor::VisitLineBox(LineBox* line) {
  PaintDecoration(*line);
  ClipScope clip_scope(this, line->content_bounds());
  for (const auto& child : line->child_boxes())
    Visit(child);
}

void PaintVisitor::VisitTextBox(TextBox* text) {
  PaintDecoration(*text);
  ClipScope clip_scope(this, text->content_bounds());
  builder_.AddNew<DrawTextDisplayItem>(text->content_bounds(), text->color(),
                                       text->baseline(), text->text());
}

//////////////////////////////////////////////////////////////////////
//
// PaintVisitor::ClipScope
//
PaintVisitor::ClipScope::ClipScope(PaintVisitor* painter,
                                   const FloatRect& bounds)
    : builder_(&painter->builder_) {
  builder_->AddNew<BeginClipDisplayItem>(bounds);
}

PaintVisitor::ClipScope::~ClipScope() {
  builder_->AddNew<EndClipDisplayItem>();
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Painter
//
Painter::Painter() {}
Painter::~Painter() {}

std::unique_ptr<DisplayItemList> Painter::Paint(const Box& box) {
  return PaintVisitor().Paint(box);
}

}  // namespace visuals
