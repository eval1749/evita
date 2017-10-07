// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <list>
#include <unordered_set>
#include <vector>

#include "evita/text/layout/paint_view_builder.h"

#include "base/logging.h"
#include "base/trace_event/trace_event.h"
#include "evita/css/selector_builder.h"
#include "evita/gfx/font.h"
#include "evita/text/layout/block_flow.h"
#include "evita/text/layout/known_names.h"
#include "evita/text/layout/line/inline_box.h"
#include "evita/text/layout/line/inline_box_visitor.h"
#include "evita/text/layout/line/root_inline_box.h"
#include "evita/text/layout/render_selection.h"
#include "evita/text/layout/text_formatter.h"
#include "evita/text/models/buffer.h"
#include "evita/text/paint/public/line/inline_box.h"
#include "evita/text/paint/public/line/root_inline_box.h"
#include "evita/text/paint/public/ruler.h"
#include "evita/text/paint/public/selection.h"
#include "evita/text/paint/public/view.h"
#include "evita/text/style/computed_style.h"
#include "evita/text/style/style_tree.h"
#include "evita/ui/animation/animatable_window.h"
#include "evita/ui/base/ime/text_input_client.h"

namespace layout {

namespace {

class PaintInlineBoxBuilder : public InlineBoxVisitor {
 public:
  PaintInlineBoxBuilder(float line_height, float line_descent);
  ~PaintInlineBoxBuilder() final = default;

  paint::InlineBox* Build(const InlineBox& box);

 private:
#define V(name) void Visit##name(layout::name* box) final;
  FOR_EACH_INLINE_BOX(V)
#undef V
  const float line_descent_;
  const float line_height_;
  paint::InlineBox* paint_box_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(PaintInlineBoxBuilder);
};

PaintInlineBoxBuilder::PaintInlineBoxBuilder(float line_height,
                                             float line_descent)
    : line_descent_(line_descent), line_height_(line_height) {}

paint::InlineBox* PaintInlineBoxBuilder::Build(const InlineBox& box) {
  paint_box_ = nullptr;
  const_cast<InlineBox&>(box).Accept(this);
  return paint_box_;
}

void PaintInlineBoxBuilder::VisitInlineFillerBox(InlineFillerBox* box) {
  DCHECK(!paint_box_);
  paint_box_ = new paint::InlineFillerBox(
      box->style(), box->width(), box->height(), line_height_, line_descent_);
}

void PaintInlineBoxBuilder::VisitInlineMarkerBox(InlineMarkerBox* box) {
  DCHECK(!paint_box_);
  paint_box_ = new paint::InlineMarkerBox(
      box->style(), box->font(), box->width(), box->height(),
      static_cast<paint::TextMarker>(box->marker_name()), line_height_,
      line_descent_);
}

void PaintInlineBoxBuilder::VisitInlineTextBox(InlineTextBox* box) {
  DCHECK(!paint_box_);
  paint_box_ = new paint::InlineTextBox(box->style(), box->font(), box->width(),
                                        box->height(), box->characters(),
                                        line_height_, line_descent_);
}

void PaintInlineBoxBuilder::VisitInlineUnicodeBox(InlineUnicodeBox* box) {
  DCHECK(!paint_box_);
  paint_box_ = new paint::InlineUnicodeBox(
      box->style(), box->font(), box->width(), box->height(), box->characters(),
      line_height_, line_descent_);
}

gfx::RectF RoundBounds(const gfx::RectF& bounds) {
  return gfx::RectF(::floor(bounds.left), ::floor(bounds.top),
                    ::floor(bounds.right), ::floor(bounds.bottom));
}

gfx::PointF CalculateSelectionStartPoint(const RootInlineBox& root_box,
                                         const TextSelection& selection) {
  if (root_box.Contains(selection.start()))
    return root_box.HitTestTextPosition(selection.start()).origin();
  return root_box.HitTestTextPosition(root_box.text_start()).origin();
}

gfx::PointF CalculateSelectionEndPoint(const RootInlineBox& root_box,
                                       const TextSelection& selection) {
  if (root_box.Contains(selection.end()))
    return root_box.HitTestTextPosition(selection.end()).bottom_left();
  return root_box.bounds().bottom_right();
}

gfx::RectF CalculateSelectionBounds(const RootInlineBox& root_box,
                                    const TextSelection& selection) {
  DCHECK(selection.is_range());
  if (selection.start() >= root_box.text_end() ||
      selection.end() <= root_box.text_start()) {
    return gfx::RectF();
  }
  return gfx::RectF(
      CalculateSelectionStartPoint(root_box, selection).x, root_box.top(),
      CalculateSelectionEndPoint(root_box, selection).x, root_box.bottom());
}

std::unordered_set<gfx::RectF> CalculateSelectionBoundsSet(
    const std::list<RootInlineBox*>& lines,
    const TextSelection& selection,
    const gfx::RectF& block_bounds) {
  std::unordered_set<gfx::RectF> bounds_set;
  if (selection.is_caret())
    return bounds_set;
  if (selection.start() >= lines.back()->text_end())
    return bounds_set;
  if (selection.end() <= lines.front()->text_start())
    return bounds_set;
  for (auto line : lines) {
    if (selection.end() <= line->text_start())
      break;
    const auto& bounds = CalculateSelectionBounds(*line, selection);
    if (bounds.empty())
      continue;
    bounds_set.insert(block_bounds.Intersect(RoundBounds(bounds)));
  }
  return bounds_set;
}

gfx::ColorF ComputeBackgroundColor(const StyleTree& style_tree) {
  css::Selector::Builder selector;
  selector.SetTagName(KNOWN_NAME_OF(cc_default));
  const auto& style = style_tree.ComputedStyleOf(selector.Build());
  return style.bgcolor();
}

paint::Ruler ComputeRuler(const StyleTree& style_tree, const BlockFlow& block) {
  // TODO(eval1749): We should expose show/hide and ruler settings to both
  // script and UI.
  css::Selector::Builder selector;
  selector.SetTagName(KNOWN_NAME_OF(cc_ruler));
  const auto& style = style_tree.ComputedStyleOf(selector.Build());
  // TODO(eval1749): We should consider zoom.
  const auto font = style.fonts()[0];
  const auto& bounds = block.bounds();
  const auto num_columns = 81;
  const auto width_of_M = font->GetCharWidth('M');
  const auto ruler_x = ::floor(bounds.left + width_of_M * num_columns);
  // TODO(eval1749): We should get width or ruler from CSS.
  const auto width = 1.0f;
  return paint::Ruler(ruler_x, width, style.color());
}

paint::RootInlineBox* CreatePaintRootInlineBox(const RootInlineBox& line) {
  TRACE_EVENT0("view", "CreatePaintRootInlineBox");
  std::vector<paint::InlineBox*> boxes;
  boxes.reserve(line.boxes().size());
  PaintInlineBoxBuilder builder(line.height(), line.descent());
  for (const auto& box : line.boxes())
    boxes.push_back(builder.Build(*box));
  return new paint::RootInlineBox(boxes, line.bounds());
}

TextSelection FormatSelection(const StyleTree& style_tree,
                              const TextSelectionModel& selection_model) {
  css::Selector::Builder selector;
  selector.SetTagName(KNOWN_NAME_OF(cc_selection));
  if (selection_model.disabled())
    selector.AddClass(KNOWN_NAME_OF(c_inactive));
  else
    selector.AddClass(KNOWN_NAME_OF(c_active));
  const auto& style = style_tree.ComputedStyleOf(selector.Build());
  return TextSelection(selection_model, style.bgcolor());
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// PaintViewBuilder
//
PaintViewBuilder::PaintViewBuilder() {}
PaintViewBuilder::~PaintViewBuilder() {}

scoped_refptr<paint::View> PaintViewBuilder::Build(
    const BlockFlow& block,
    const TextSelectionModel& selection_model,
    const CaretDisplayItem& caret) {
  TRACE_EVENT0("view", "PaintViewBuilder::Build");
  // TODO(eval1749): We should recompute default style when style is changed,
  // rather than every |Format| call.
  const auto& bgcolor = ComputeBackgroundColor(block.style_tree());
  const auto& ruler = ComputeRuler(block.style_tree(), block);
  const auto& selection = FormatSelection(block.style_tree(), selection_model);
  const auto& selection_bounds_set =
      CalculateSelectionBoundsSet(block.lines(), selection, block.bounds());

  std::vector<paint::RootInlineBox*> lines;
  lines.reserve(block.lines().size());
  for (const auto& line : block.lines())
    lines.push_back(CreatePaintRootInlineBox(*line));
  return new paint::View(block.version(), block.bounds(), lines,
                         base::WrapRefCounted(new paint::Selection(
                             selection.color(), selection_bounds_set)),
                         bgcolor, ruler, caret);
}

}  // namespace layout
