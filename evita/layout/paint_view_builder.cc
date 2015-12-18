// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <list>
#include <vector>

#include "evita/layout/paint_view_builder.h"

#include "base/logging.h"
#include "evita/gfx/font.h"
#include "evita/text/buffer.h"
#include "evita/ui/animation/animatable_window.h"
#include "evita/ui/base/ime/text_input_client.h"
#include "evita/layout/layout_block_flow.h"
#include "evita/layout/line/inline_box.h"
#include "evita/layout/line/inline_box_visitor.h"
#include "evita/layout/line/root_inline_box.h"
#include "evita/layout/render_font_set.h"
#include "evita/layout/render_selection.h"
#include "evita/layout/render_style.h"
#include "evita/layout/text_formatter.h"
#include "evita/paint/public/line/inline_box.h"
#include "evita/paint/public/line/root_inline_box.h"
#include "evita/paint/public/selection.h"
#include "evita/paint/public/view.h"

namespace layout {

namespace {
const auto kBlinkInterval = 16 * 20;  // milliseconds

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
      box->style(), box->width(), box->height(),
      static_cast<paint::TextMarker>(box->marker_name()), line_height_,
      line_descent_);
}

void PaintInlineBoxBuilder::VisitInlineTextBox(InlineTextBox* box) {
  DCHECK(!paint_box_);
  paint_box_ =
      new paint::InlineTextBox(box->style(), box->width(), box->height(),
                               box->characters(), line_height_, line_descent_);
}

void PaintInlineBoxBuilder::VisitInlineUnicodeBox(InlineUnicodeBox* box) {
  DCHECK(!paint_box_);
  paint_box_ = new paint::InlineUnicodeBox(box->style(), box->width(),
                                           box->height(), box->characters(),
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
    const std::list<scoped_refptr<RootInlineBox>>& lines,
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

paint::RootInlineBox* CreatePaintRootInlineBox(const RootInlineBox& line) {
  std::vector<paint::InlineBox*> boxes;
  boxes.reserve(line.boxes().size());
  PaintInlineBoxBuilder builder(line.height(), line.descent());
  for (const auto& box : line.boxes())
    boxes.push_back(builder.Build(*box));
  return new paint::RootInlineBox(boxes, line.bounds());
}

base::TimeDelta GetCaretBlinkInterval() {
  const auto interval = ::GetCaretBlinkTime();
  if (!interval)
    return base::TimeDelta::FromMilliseconds(kBlinkInterval);
  if (interval == INFINITE)
    return base::TimeDelta();
  return base::TimeDelta::FromMilliseconds(interval);
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// PaintViewBuilder
//
PaintViewBuilder::PaintViewBuilder(const LayoutBlockFlow& block,
                                   ui::AnimatableWindow* caret_owner)
    : block_(block),
      caret_owner_(caret_owner),
      caret_state_(paint::CaretState::None),
      zoom_(1.0f) {}

PaintViewBuilder::~PaintViewBuilder() {
  caret_timer_.Stop();
}

scoped_refptr<paint::View> PaintViewBuilder::Build(
    const TextSelectionModel& selection_model,
    base::Time now) {
  // TODO(eval1749): We should recompute default style when style is changed,
  // rather than every |Format| call.
  const auto& bgcolor =
      ColorToColorF(block_.text_buffer().GetDefaultStyle().bgcolor());
  const auto& ruler_bounds = ComputeRulerBounds();
  const auto& selection =
      TextFormatter::FormatSelection(block_.text_buffer(), selection_model);
  const auto& caret_bounds = ComputeCaretBounds(selection_model);
  const auto caret_state = ComputeCaretState(caret_bounds, now);

  if (caret_bounds.empty()) {
    StopCaretTimer();
  } else if (caret_bounds_ != caret_bounds) {
    ui::TextInputClient::Get()->set_caret_bounds(caret_bounds);
    caret_time_ = now;
    StartCaretTimer();
  }

  caret_bounds_ = caret_bounds;
  caret_state_ = caret_state;

  const auto& selection_bounds_set =
      CalculateSelectionBoundsSet(block_.lines(), selection, block_.bounds());

  std::vector<paint::RootInlineBox*> lines;
  lines.reserve(block_.lines().size());
  for (const auto& line : block_.lines())
    lines.push_back(CreatePaintRootInlineBox(*line));
  return new paint::View(block_.format_counter(), block_.bounds(), lines,
                         make_scoped_refptr(new paint::Selection(
                             selection.color(), selection_bounds_set)),
                         bgcolor, ruler_bounds, std::make_unique<paint::Caret>(
                                                    caret_state, caret_bounds));
}

gfx::RectF PaintViewBuilder::ComputeCaretBounds(
    const TextSelectionModel& selection_model) const {
  if (!selection_model.has_focus())
    return gfx::RectF();
  const auto& char_rect =
      RoundBounds(block_.HitTestTextPosition(selection_model.focus_offset()));
  if (char_rect.empty())
    return gfx::RectF();
  const auto caret_width = 2;
  return gfx::RectF(char_rect.left, char_rect.top, char_rect.left + caret_width,
                    char_rect.bottom);
}

paint::CaretState PaintViewBuilder::ComputeCaretState(const gfx::RectF& bounds,
                                                      base::Time now) const {
  if (bounds.empty())
    return paint::CaretState::None;

  if (caret_state_ == paint::CaretState::None) {
    // This view starts showing caret.
    return paint::CaretState::Show;
  }

  if (caret_bounds_ != bounds) {
    // The caret is moved.
    return paint::CaretState::Show;
  }

  // When the caret stays at same point, caret is blinking.
  const auto interval = GetCaretBlinkInterval();
  if (interval == base::TimeDelta())
    return paint::CaretState::Show;
  const auto delta = now - caret_time_;
  const auto index = delta / interval;
  return index % 2 ? paint::CaretState::Hide : paint::CaretState::Show;
}

gfx::RectF PaintViewBuilder::ComputeRulerBounds() const {
  // TODO(eval1749): We should expose show/hide and ruler settings to both
  // script and UI.
  auto style = block_.text_buffer().GetDefaultStyle();
  style.set_font_size(style.font_size() * zoom_);
  const auto font = FontSet::GetFont(style, 'x');

  const auto num_columns = 81;
  const auto width_of_M = font->GetCharWidth('M');
  const auto ruler_x = ::floor(bounds_.left + width_of_M * num_columns);
  return gfx::RectF(gfx::PointF(ruler_x, bounds_.top),
                    gfx::SizeF(1.0f, bounds_.height()));
}

void PaintViewBuilder::DidFireCaretTimer() {
  caret_owner_->RequestAnimationFrame();
}

void PaintViewBuilder::SetBounds(const gfx::RectF& new_bounds) {
  bounds_ = new_bounds;
  caret_bounds_ = gfx::RectF();
}

void PaintViewBuilder::SetZoom(float new_zoom) {
  DCHECK_GT(new_zoom, 0.0f);
  zoom_ = new_zoom;
}

void PaintViewBuilder::StartCaretTimer() {
  const auto interval = GetCaretBlinkInterval();
  if (interval == base::TimeDelta())
    return;
  caret_timer_.Start(
      FROM_HERE, interval,
      base::Bind(&PaintViewBuilder::DidFireCaretTimer, base::Unretained(this)));
}

void PaintViewBuilder::StopCaretTimer() {
  caret_timer_.Stop();
}

}  // namespace layout
