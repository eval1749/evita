// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/paint/view_painter.h"

#include "base/trace_event/trace_event.h"
#include "evita/gfx_base.h"
#include "evita/paint/root_inline_box_list_painter.h"
#include "evita/paint/view_paint_cache.h"
#include "evita/views/text/layout_caret.h"
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

std::unordered_set<gfx::RectF> CalculateSelectionBoundsList(
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

std::unique_ptr<ViewPaintCache> ViewPainter::Paint(
    gfx::Canvas* canvas,
    std::unique_ptr<ViewPaintCache> view_cache) {
  if (view_cache && !view_cache->NeedsTextPaint(canvas, layout_view_)) {
    PaintSelectionWithCache(canvas, *view_cache);
    view_cache->UpdateSelection(layout_view_.selection(), caret_bounds_);
    return std::move(view_cache);
  }
  const auto& cached_lines = view_cache && view_cache->CanUseTextImage(canvas)
                                 ? view_cache->lines()
                                 : std::vector<RootInlineBox*>();
  paint::RootInlineBoxListPainter painter(canvas, layout_view_.bounds(),
                                          layout_view_.bgcolor(),
                                          layout_view_.lines(), cached_lines);
  if (view_cache)
    RestoreCaretBackgroundIfNeeded(canvas, *view_cache);
  if (!painter.Paint()) {
    TRACE_EVENT0("view", "ViewPainter::PaintClean");
    PaintSelection(canvas);
    PaintCaretIfNeeded(canvas);
    view_cache->UpdateSelection(layout_view_.selection(), caret_bounds_);
    return std::move(view_cache);
  }

  TRACE_EVENT0("view", "ViewPainter::PaintDirty");
  canvas->SaveScreenImage(layout_view_.bounds());
  painter.Finish();
  PaintSelection(canvas);
  PaintRuler(canvas);
  PaintCaretIfNeeded(canvas);
  return std::make_unique<ViewPaintCache>(canvas, layout_view_, caret_bounds_);
}

void ViewPainter::PaintCaretIfNeeded(gfx::Canvas* canvas) {
  if (!layout_view_.caret().is_show())
    return;
  caret_bounds_ = layout_view_.caret().bounds();
  DCHECK(!caret_bounds_.empty());
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, caret_bounds_);
  canvas->AddDirtyRect(caret_bounds_);
  canvas->Clear(gfx::ColorF::Black);
}

void ViewPainter::PaintSelection(gfx::Canvas* canvas) {
  const auto& selection = layout_view_.selection();
  if (!selection.is_range())
    return;
  TRACE_EVENT0("view", "ViewPainter::PaintSelection");
  const auto& lines = layout_view_.lines();
  if (selection.start() >= lines.back()->text_end()) {
    // The selection starts after the view.
    return;
  }

  if (selection.end() <= lines.front()->text_start()) {
    // The selection ends before the view.
    return;
  }

  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, layout_view_.bounds());
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

void ViewPainter::PaintSelectionWithCache(gfx::Canvas* canvas,
                                          const ViewPaintCache& view_cache) {
  DCHECK(!view_cache.NeedsTextPaint(canvas, layout_view_));
  const auto& new_selection = layout_view_.selection();
  const auto& old_selection = view_cache.selection();
  const auto& bounds = layout_view_.bounds();
  const auto& lines = view_cache.lines();
  auto new_selection_bounds_list =
      CalculateSelectionBoundsList(lines, new_selection, bounds);
  auto old_selection_bounds_list =
      CalculateSelectionBoundsList(lines, old_selection, bounds);

  gfx::Canvas::DrawingScope drawing_scope(canvas);
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, bounds);
  for (const auto& old_rect : old_selection_bounds_list) {
    if (new_selection_bounds_list.find(old_rect) !=
        new_selection_bounds_list.end())
      continue;
    canvas->AddDirtyRect(old_rect);
    canvas->RestoreScreenImage(old_rect);
  }
  RestoreCaretBackgroundIfNeeded(canvas, view_cache);

  if (old_selection.color() != new_selection.color())
    old_selection_bounds_list.clear();
  if (!new_selection_bounds_list.empty()) {
    gfx::Brush fill_brush(canvas, new_selection.color());
    for (const auto& new_rect : new_selection_bounds_list) {
      if (old_selection_bounds_list.find(new_rect) !=
          old_selection_bounds_list.end())
        continue;
      canvas->AddDirtyRect(new_rect);
      canvas->RestoreScreenImage(new_rect);
      canvas->FillRectangle(fill_brush, new_rect);
    }
  }
  PaintCaretIfNeeded(canvas);
}

void ViewPainter::PaintRuler(gfx::Canvas* canvas) {
  const auto& ruler_bounds = layout_view_.ruler_bounds();
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, ruler_bounds);
  // TODO(eval1749): We should get ruler color from CSS.
  gfx::Brush brush(canvas, gfx::ColorF(0, 0, 0, 0.3f));
  canvas->DrawRectangle(brush, ruler_bounds);
}

void ViewPainter::RestoreCaretBackgroundIfNeeded(
    gfx::Canvas* canvas,
    const ViewPaintCache& view_cache) {
  const auto& bounds = view_cache.caret_bounds();
  if (bounds.empty())
    return;
  canvas->AddDirtyRect(bounds);
  canvas->RestoreScreenImage(bounds);
}

}  // namespace pain
