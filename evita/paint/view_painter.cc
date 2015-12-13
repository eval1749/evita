// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/paint/view_painter.h"

#include "base/trace_event/trace_event.h"
#include "evita/gfx_base.h"
#include "evita/paint/root_inline_box_list_painter.h"
#include "evita/paint/view_paint_cache.h"
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
ViewPainter::ViewPainter(const LayoutView& layout_view)
    : layout_view_(layout_view) {}

ViewPainter::~ViewPainter() {}

ui::Caret* ViewPainter::caret() const {
  return layout_view_.caret();
}

std::unique_ptr<ViewPaintCache> ViewPainter::Paint(
    gfx::Canvas* canvas,
    base::Time now,
    std::unique_ptr<ViewPaintCache> view_cache) {
  if (view_cache && !view_cache->NeedsTextPaint(canvas, layout_view_)) {
    PaintSelectionIfNeeded(canvas, now, *view_cache);
    view_cache->UpdateSelection(layout_view_.selection());
    return std::move(view_cache);
  }
  paint::RootInlineBoxListPainter painter(
      canvas, layout_view_.bounds(), layout_view_.bgcolor(),
      layout_view_.lines(), view_cache && view_cache->CanUseTextImage(canvas)
                                ? view_cache->lines()
                                : std::vector<RootInlineBox*>{});
  caret()->DidPaint(layout_view_.bounds());
  if (!painter.Paint()) {
    TRACE_EVENT0("view", "ViewPainter::PaintClean");
    PaintSelection(canvas, now);
    view_cache->UpdateSelection(layout_view_.selection());
    return std::move(view_cache);
  }

  TRACE_EVENT0("view", "ViewPainter::PaintDirty");
  canvas->SaveScreenImage(layout_view_.bounds());
  painter.Finish();
  PaintSelection(canvas, now);
  PaintRuler(canvas);
  return std::make_unique<ViewPaintCache>(canvas, layout_view_);
}

void ViewPainter::PaintSelection(gfx::Canvas* canvas, base::Time now) {
  TRACE_EVENT0("view", "ViewPainter::PaintSelection");
  const auto& lines = layout_view_.lines();
  const auto& selection = layout_view_.selection();
  if (selection.start() >= lines.back()->text_end()) {
    caret()->Hide(canvas);
    return;
  }
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, layout_view_.bounds());
  if (selection.is_range() && selection.end() > lines.front()->text_start()) {
    gfx::Brush fill_brush(canvas, selection.color());
    for (auto line : lines) {
      if (selection.end() <= line->text_start())
        break;
      auto const rect = line->CalculateSelectionRect(selection);
      if (rect.empty())
        continue;
      canvas->FillRectangle(fill_brush, rect);
    }
  }
  UpdateCaret(canvas, now);
}

void ViewPainter::PaintSelectionIfNeeded(gfx::Canvas* canvas,
                                         base::Time now,
                                         const ViewPaintCache& view_cache) {
  DCHECK(!view_cache.NeedsTextPaint(canvas, layout_view_));
  const auto& new_selection = layout_view_.selection();
  const auto& old_selection = view_cache.selection();
  if (old_selection == new_selection) {
    if (!old_selection.has_focus())
      return;
    caret()->Blink(canvas, now);
    return;
  }
  const auto& bounds = layout_view_.bounds();
  const auto& lines = view_cache.lines();
  auto new_old_selection_rects =
      CalculateSelectionRects(lines, new_selection, bounds);
  auto old_old_selection_rects =
      CalculateSelectionRects(lines, old_selection, bounds);

  gfx::Canvas::DrawingScope drawing_scope(canvas);
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, bounds);
  for (const auto& old_rect : old_old_selection_rects) {
    if (new_old_selection_rects.find(old_rect) != new_old_selection_rects.end())
      continue;
    canvas->AddDirtyRect(old_rect);
    canvas->RestoreScreenImage(old_rect);
    caret()->DidPaint(old_rect);
  }

  if (old_selection.color() != new_selection.color())
    old_old_selection_rects.clear();
  if (!new_old_selection_rects.empty()) {
    gfx::Brush fill_brush(canvas, new_selection.color());
    for (const auto& new_rect : new_old_selection_rects) {
      if (old_old_selection_rects.find(new_rect) !=
          old_old_selection_rects.end())
        continue;
      canvas->AddDirtyRect(new_rect);
      canvas->RestoreScreenImage(new_rect);
      canvas->FillRectangle(fill_brush, new_rect);
      caret()->DidPaint(new_rect);
    }
  }

  caret()->Hide(canvas);
  UpdateCaret(canvas, now);
}

void ViewPainter::UpdateCaret(gfx::Canvas* canvas, base::Time now) {
  DCHECK(!caret()->visible());
  const auto& selection = layout_view_.selection();
  if (!selection.has_focus())
    return;
  auto const char_rect =
      RoundBounds(layout_view_.HitTestTextPosition(selection.focus_offset()));
  if (char_rect.empty())
    return;
  auto const caret_width = 2;
  gfx::RectF caret_bounds(char_rect.left, char_rect.top,
                          char_rect.left + caret_width, char_rect.bottom);
  caret()->Update(canvas, now, caret_bounds);
}

void ViewPainter::PaintRuler(gfx::Canvas* canvas) {
  const auto& ruler_bounds = layout_view_.ruler_bounds();
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, ruler_bounds);
  // TODO(eval1749): We should get ruler color from CSS.
  gfx::Brush brush(canvas, gfx::ColorF(0, 0, 0, 0.3f));
  canvas->DrawRectangle(brush, ruler_bounds);
}

}  // namespace pain
