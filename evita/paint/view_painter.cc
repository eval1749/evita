// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/paint/view_painter.h"

#include "base/trace_event/trace_event.h"
#include "evita/gfx_base.h"
#include "evita/paint/root_inline_box_list_painter.h"
#include "evita/ui/caret.h"
#include "evita/views/text/layout_view.h"
#include "evita/views/text/render_selection.h"
#include "evita/views/text/root_inline_box.h"

namespace paint {

using RootInlineBox = views::rendering::RootInlineBox;
using TextSelection = views::rendering::TextSelection;

namespace {

gfx::RectF RoundBounds(const gfx::RectF& bounds) {
  return gfx::RectF(::floor(bounds.left), ::floor(bounds.top),
                    ::floor(bounds.right), ::floor(bounds.bottom));
}

std::unordered_set<gfx::RectF> CalculateSelectionRects(
    const std::vector<RootInlineBox*>& lines,
    const TextSelection& selection,
    const gfx::RectF& bounds) {
  std::unordered_set<gfx::RectF> rects;
  if (selection.is_caret())
    return rects;
  if (selection.start() >= lines.back()->text_end())
    return rects;
  if (selection.end() <= lines.front()->text_start())
    return rects;
  for (auto line : lines) {
    if (selection.end() <= line->text_start())
      break;
    auto const rect = line->CalculateSelectionRect(selection);
    if (rect.empty())
      continue;
    rects.insert(bounds.Intersect(RoundBounds(rect)));
  }
  return rects;
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// ViewPainter
//
ViewPainter::ViewPainter(gfx::Canvas* canvas,
                         base::Time now,
                         ui::Caret* caret,
                         const LayoutView* last_layout_view)
    : caret_(caret),
      canvas_(canvas),
      last_layout_view_(last_layout_view),
      now_(now) {}

ViewPainter::~ViewPainter() {}

void ViewPainter::Paint(const LayoutView& layout_view) {
  if (last_layout_view_ &&
      last_layout_view_->layout_version() == layout_view.layout_version()) {
    PaintSelectionIfNeeded(layout_view);
    return;
  }
  paint::RootInlineBoxListPainter painter(
      canvas_, layout_view.bounds(), layout_view.bgcolor(), layout_view.lines(),
      last_layout_view_ ? last_layout_view_->lines()
                        : std::vector<RootInlineBox*>{});
  caret_->DidPaint(layout_view.bounds());
  if (!painter.Paint()) {
    TRACE_EVENT0("view", "ViewPainter::PaintClean");
    PaintSelection(layout_view);
    return;
  }

  TRACE_EVENT0("view", "ViewPainter::PaintDirty");
  canvas_->SaveScreenImage(layout_view.bounds());
  painter.Finish();
  PaintSelection(layout_view);
  PaintRuler(layout_view);
}

void ViewPainter::PaintSelection(const LayoutView& layout_view) {
  TRACE_EVENT0("view", "ViewPainter::PaintSelection");
  const auto& lines = layout_view.lines();
  const auto& selection = layout_view.selection();
  if (selection.start() >= lines.back()->text_end()) {
    caret_->Hide(canvas_);
    return;
  }
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas_, layout_view.bounds());
  if (selection.is_range() && selection.end() > lines.front()->text_start()) {
    gfx::Brush fill_brush(canvas_, selection.color());
    for (auto line : lines) {
      if (selection.end() <= line->text_start())
        break;
      auto const rect = line->CalculateSelectionRect(selection);
      if (rect.empty())
        continue;
      canvas_->FillRectangle(fill_brush, rect);
    }
  }
  UpdateCaret(layout_view);
}

void ViewPainter::PaintSelectionIfNeeded(const LayoutView& layout_view) {
  const auto& new_selection = layout_view.selection();
  const auto& old_selection = last_layout_view_->selection();
  if (old_selection == new_selection) {
    if (!old_selection.has_focus())
      return;
    caret_->Blink(canvas_, now_);
    return;
  }
  const auto& bounds = last_layout_view_->bounds();
  const auto& lines = last_layout_view_->lines();
  auto new_old_selection_rects =
      CalculateSelectionRects(lines, new_selection, bounds);
  auto old_old_selection_rects =
      CalculateSelectionRects(lines, old_selection, bounds);

  gfx::Canvas::DrawingScope drawing_scope(canvas_);
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas_, bounds);
  for (const auto& old_rect : old_old_selection_rects) {
    if (new_old_selection_rects.find(old_rect) != new_old_selection_rects.end())
      continue;
    canvas_->AddDirtyRect(old_rect);
    canvas_->RestoreScreenImage(old_rect);
    caret_->DidPaint(old_rect);
  }

  if (old_selection.color() != new_selection.color())
    old_old_selection_rects.clear();
  if (!new_old_selection_rects.empty()) {
    gfx::Brush fill_brush(canvas_, new_selection.color());
    for (const auto& new_rect : new_old_selection_rects) {
      if (old_old_selection_rects.find(new_rect) !=
          old_old_selection_rects.end())
        continue;
      canvas_->AddDirtyRect(new_rect);
      canvas_->RestoreScreenImage(new_rect);
      canvas_->FillRectangle(fill_brush, new_rect);
      caret_->DidPaint(new_rect);
    }
  }

  caret_->Hide(canvas_);
  UpdateCaret(layout_view);
}

void ViewPainter::UpdateCaret(const LayoutView& layout_view) {
  DCHECK(!caret_->visible());
  const auto& selection = layout_view.selection();
  if (!selection.has_focus())
    return;
  auto const char_rect =
      RoundBounds(layout_view.HitTestTextPosition(selection.focus_offset()));
  if (char_rect.empty())
    return;
  auto const caret_width = 2;
  gfx::RectF caret_bounds(char_rect.left, char_rect.top,
                          char_rect.left + caret_width, char_rect.bottom);
  caret_->Update(canvas_, now_, caret_bounds);
}

void ViewPainter::PaintRuler(const LayoutView& layout_view) {
  const auto& ruler_bounds = layout_view.ruler_bounds();
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas_, ruler_bounds);
  // TODO(eval1749): We should get ruler color from CSS.
  gfx::Brush brush(canvas_, gfx::ColorF(0, 0, 0, 0.3f));
  canvas_->DrawRectangle(brush, ruler_bounds);
}

}  // namespace pain
