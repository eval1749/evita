// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <list>
#include <vector>

#include "evita/layout/layout_view_builder.h"

#include "base/logging.h"
#include "evita/text/buffer.h"
#include "evita/ui/animation/animatable_window.h"
#include "evita/ui/base/ime/text_input_client.h"
#include "evita/layout/layout_block_flow.h"
#include "evita/layout/layout_view.h"
#include "evita/layout/line/root_inline_box.h"
#include "evita/layout/render_font.h"
#include "evita/layout/render_font_set.h"
#include "evita/layout/render_selection.h"
#include "evita/layout/render_style.h"
#include "evita/layout/text_formatter.h"
#include "evita/paint/selection.h"

namespace layout {

namespace {
const auto kBlinkInterval = 16 * 20;  // milliseconds

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
  return gfx::RectF(CalculateSelectionStartPoint(root_box, selection),
                    CalculateSelectionEndPoint(root_box, selection));
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

base::TimeDelta GetCaretBlinkInterval() {
  auto const interval = ::GetCaretBlinkTime();
  if (!interval)
    return base::TimeDelta::FromMilliseconds(kBlinkInterval);
  if (interval == INFINITE)
    return base::TimeDelta();
  return base::TimeDelta::FromMilliseconds(interval);
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// LayoutViewBuilder
//
LayoutViewBuilder::LayoutViewBuilder(const text::Buffer* buffer,
                                     ui::AnimatableWindow* caret_owner)
    : buffer_(buffer),
      caret_owner_(caret_owner),
      caret_state_(paint::CaretState::None),
      zoom_(1.0f) {}

LayoutViewBuilder::~LayoutViewBuilder() {
  caret_timer_.Stop();
}

scoped_refptr<LayoutView> LayoutViewBuilder::Build(
    const LayoutBlockFlow& layout_block_flow,
    const TextSelectionModel& selection_model,
    base::Time now) {
  // TODO(eval1749): We should recompute default style when style is changed,
  // rather than every |Format| call.
  const auto& bgcolor = ColorToColorF(buffer_->GetDefaultStyle().bgcolor());
  const auto& ruler_bounds = ComputeRulerBounds();
  const auto& selection =
      TextFormatter::FormatSelection(buffer_, selection_model);
  const auto& caret_bounds =
      ComputeCaretBounds(layout_block_flow, selection_model);
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

  const auto& selection_bounds_set = CalculateSelectionBoundsSet(
      layout_block_flow.lines(), selection, layout_block_flow.bounds());

  std::vector<RootInlineBox*> lines;
  for (const auto& line : layout_block_flow.lines())
    lines.push_back(line->Copy());
  return new LayoutView(
      layout_block_flow.format_counter(), layout_block_flow.bounds(), lines,
      make_scoped_refptr(
          new paint::Selection(selection.color(), selection_bounds_set)),
      bgcolor, ruler_bounds,
      std::make_unique<paint::Caret>(caret_state, caret_bounds));
}

gfx::RectF LayoutViewBuilder::ComputeCaretBounds(
    const LayoutBlockFlow& layout_block_flow,
    const TextSelectionModel& selection_model) const {
  if (!selection_model.has_focus())
    return gfx::RectF();
  auto const& char_rect = RoundBounds(
      layout_block_flow.HitTestTextPosition(selection_model.focus_offset()));
  if (char_rect.empty())
    return gfx::RectF();
  auto const caret_width = 2;
  return gfx::RectF(char_rect.left, char_rect.top, char_rect.left + caret_width,
                    char_rect.bottom);
}

paint::CaretState LayoutViewBuilder::ComputeCaretState(
    const gfx::RectF& bounds,
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
  auto const interval = GetCaretBlinkInterval();
  if (interval == base::TimeDelta())
    return paint::CaretState::Show;
  auto const delta = now - caret_time_;
  auto const index = delta / interval;
  return index % 2 ? paint::CaretState::Hide : paint::CaretState::Show;
}

gfx::RectF LayoutViewBuilder::ComputeRulerBounds() const {
  // TODO(eval1749): We should expose show/hide and ruler settings to both
  // script and UI.
  auto style = buffer_->GetDefaultStyle();
  style.set_font_size(style.font_size() * zoom_);
  auto const font = FontSet::GetFont(style, 'x');

  auto const num_columns = 81;
  auto const width_of_M = font->GetCharWidth('M');
  auto const ruler_x = ::floor(bounds_.left + width_of_M * num_columns);
  return gfx::RectF(gfx::PointF(ruler_x, bounds_.top),
                    gfx::SizeF(1.0f, bounds_.height()));
}

void LayoutViewBuilder::DidFireCaretTimer() {
  caret_owner_->RequestAnimationFrame();
}

void LayoutViewBuilder::SetBounds(const gfx::RectF& new_bounds) {
  bounds_ = new_bounds;
  caret_bounds_ = gfx::RectF();
}

void LayoutViewBuilder::SetZoom(float new_zoom) {
  DCHECK_GT(new_zoom, 0.0f);
  zoom_ = new_zoom;
}

void LayoutViewBuilder::StartCaretTimer() {
  const auto interval = GetCaretBlinkInterval();
  if (interval == base::TimeDelta())
    return;
  caret_timer_.Start(FROM_HERE, interval,
                     base::Bind(&LayoutViewBuilder::DidFireCaretTimer,
                                base::Unretained(this)));
}

void LayoutViewBuilder::StopCaretTimer() {
  caret_timer_.Stop();
}

}  // namespace layout
