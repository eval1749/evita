// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <unordered_set>

#include "evita/views/text/screen_text_block.h"

#include "base/logging.h"
#include "base/time/time.h"
#include "base/trace_event/trace_event.h"
#include "evita/gfx/bitmap.h"
#include "evita/paint/layout_view_painter.h"
#include "evita/paint/root_inline_box_list_painter.h"
#include "evita/ui/caret.h"
#include "evita/views/text/layout_view.h"
#include "evita/views/text/render_selection.h"
#include "evita/views/text/root_inline_box.h"

namespace views {
namespace rendering {

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
// ScreenTextBlock
//
ScreenTextBlock::ScreenTextBlock(ui::Caret* caret) : caret_(caret) {}

ScreenTextBlock::~ScreenTextBlock() {}

void ScreenTextBlock::Paint(gfx::Canvas* canvas,
                            base::Time now,
                            const LayoutView* last_layout_view,
                            const LayoutView& layout_view) {
  if (last_layout_view &&
      last_layout_view->layout_version() == layout_view.layout_version()) {
    PaintSelectionIfNeeded(canvas, now, last_layout_view, layout_view);
    return;
  }
  paint::RootInlineBoxListPainter painter(
      canvas, layout_view.bounds(), layout_view.bgcolor(), layout_view.lines(),
      last_layout_view ? last_layout_view->lines()
                       : std::vector<RootInlineBox*>{});
  caret_->DidPaint(layout_view.bounds());
  if (!painter.Paint()) {
    TRACE_EVENT0("view", "ScreenTextBlock::PaintClean");
    PaintSelection(canvas, now, layout_view);
    return;
  }

  TRACE_EVENT0("view", "ScreenTextBlock::PaintDirty");
  canvas->SaveScreenImage(layout_view.bounds());
  painter.Finish();
  PaintSelection(canvas, now, layout_view);
  paint::LayoutViewPainter(canvas).Paint(layout_view);
}

void ScreenTextBlock::PaintSelection(gfx::Canvas* canvas,
                                     base::Time now,
                                     const LayoutView& layout_view) {
  TRACE_EVENT0("view", "ScreenTextBlock::PaintSelection");
  const auto& lines = layout_view.lines();
  const auto& selection = layout_view.selection();
  if (selection.start() >= lines.back()->text_end()) {
    caret_->Hide(canvas);
    return;
  }
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, layout_view.bounds());
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
  UpdateCaret(canvas, now, layout_view);
}

void ScreenTextBlock::PaintSelectionIfNeeded(gfx::Canvas* canvas,
                                             base::Time now,
                                             const LayoutView* last_layout_view,
                                             const LayoutView& layout_view) {
  const auto& new_selection = layout_view.selection();
  const auto& old_selection = last_layout_view->selection();
  if (old_selection == new_selection) {
    if (!old_selection.has_focus())
      return;
    caret_->Blink(canvas, now);
    return;
  }
  const auto& bounds = last_layout_view->bounds();
  const auto& lines = last_layout_view->lines();
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
    caret_->DidPaint(old_rect);
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
      caret_->DidPaint(new_rect);
    }
  }

  caret_->Hide(canvas);
  UpdateCaret(canvas, now, layout_view);
}

void ScreenTextBlock::UpdateCaret(gfx::Canvas* canvas,
                                  base::Time now,
                                  const LayoutView& layout_view) {
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
  caret_->Update(canvas, now, caret_bounds);
}

}  // namespace rendering
}  // namespace views
