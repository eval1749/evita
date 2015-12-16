// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "evita/paint/view_painter.h"

#include "base/trace_event/trace_event.h"
#include "evita/gfx_base.h"
#include "evita/layout/layout_view.h"
#include "evita/layout/line/root_inline_box.h"
#include "evita/paint/caret.h"
#include "evita/paint/root_inline_box_list_painter.h"
#include "evita/paint/selection.h"
#include "evita/paint/view_paint_cache.h"

namespace paint {

using RootInlineBox = layout::RootInlineBox;
using TextSelection = layout::TextSelection;

namespace {

gfx::RectF RoundBounds(const gfx::RectF& bounds) {
  return gfx::RectF(::floor(bounds.left), ::floor(bounds.top),
                    ::floor(bounds.right), ::floor(bounds.bottom));
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
  TRACE_EVENT0("view", "ViewPainter::Paint");
  if (view_cache && !view_cache->NeedsTextPaint(canvas, layout_view_)) {
    TRACE_EVENT0("view", "ViewPainter::Paint.Selection");
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
    TRACE_EVENT0("view", "ViewPainter::Paint.Clean");
    PaintSelection(canvas);
    view_cache->UpdateSelection(layout_view_.selection(), caret_bounds_);
    return std::move(view_cache);
  }

  TRACE_EVENT0("view", "ViewPainter::Paint.Dirty");
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

void ViewPainter::PaintRuler(gfx::Canvas* canvas) {
  const auto& ruler_bounds = layout_view_.ruler_bounds();
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, ruler_bounds);
  // TODO(eval1749): We should get ruler color from CSS.
  gfx::Brush brush(canvas, gfx::ColorF(0, 0, 0, 0.3f));
  canvas->DrawRectangle(brush, ruler_bounds);
}

void ViewPainter::PaintSelection(gfx::Canvas* canvas) {
  const auto& selection = *layout_view_.selection();
  if (selection.bounds_set().empty())
    return;
  TRACE_EVENT0("view", "ViewPainter::PaintSelection");
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, layout_view_.bounds());
  gfx::Brush fill_brush(canvas, selection.color());
  for (auto bounds : selection.bounds_set())
    canvas->FillRectangle(fill_brush, bounds);
}

void ViewPainter::PaintSelectionWithCache(gfx::Canvas* canvas,
                                          const ViewPaintCache& view_cache) {
  DCHECK(!view_cache.NeedsTextPaint(canvas, layout_view_));
  const auto& new_selection = *layout_view_.selection();
  const auto& old_selection = view_cache.selection();

  const auto& bounds = layout_view_.bounds();
  gfx::Canvas::DrawingScope drawing_scope(canvas);
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, bounds);

  if (old_selection.color() == new_selection.color()) {
    for (const auto& old_bounds : old_selection.bounds_set()) {
      if (new_selection.HasBounds(old_bounds))
        continue;
      canvas->AddDirtyRect(old_bounds);
      canvas->RestoreScreenImage(old_bounds);
    }
    RestoreCaretBackgroundIfNeeded(canvas, view_cache);
    if (!new_selection.bounds_set().empty()) {
      gfx::Brush fill_brush(canvas, new_selection.color());
      for (const auto& new_bounds : new_selection.bounds_set()) {
        if (old_selection.HasBounds(new_bounds))
          continue;
        canvas->AddDirtyRect(new_bounds);
        canvas->RestoreScreenImage(new_bounds);
        canvas->FillRectangle(fill_brush, new_bounds);
      }
    }
    PaintCaretIfNeeded(canvas);
    return;
  }

  for (const auto& old_bounds : old_selection.bounds_set()) {
    canvas->AddDirtyRect(old_bounds);
    canvas->RestoreScreenImage(old_bounds);
  }
  RestoreCaretBackgroundIfNeeded(canvas, view_cache);
  if (!new_selection.bounds_set().empty()) {
    gfx::Brush fill_brush(canvas, new_selection.color());
    for (const auto& new_bounds : new_selection.bounds_set()) {
      canvas->AddDirtyRect(new_bounds);
      canvas->FillRectangle(fill_brush, new_bounds);
    }
  }
  PaintCaretIfNeeded(canvas);
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

}  // namespace paint
