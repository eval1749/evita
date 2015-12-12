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
// ScreenTextBlock::Caret
//
class ScreenTextBlock::Caret final : public ui::Caret {
 public:
  explicit Caret(ui::CaretOwner* owner);
  ~Caret() final = default;

 private:
  void Paint(gfx::Canvas* canvas, const gfx::RectF& bounds) final;

  DISALLOW_COPY_AND_ASSIGN(Caret);
};

ScreenTextBlock::Caret::Caret(ui::CaretOwner* owner) : ui::Caret(owner) {}

void ScreenTextBlock::Caret::Paint(gfx::Canvas* canvas,
                                   const gfx::RectF& bounds) {
  if (visible()) {
    gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, bounds);
    canvas->AddDirtyRect(bounds);
    canvas->Clear(gfx::ColorF::Black);
    return;
  }
  if (!canvas->screen_bitmap())
    return;
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, bounds);
  canvas->AddDirtyRect(bounds);
  canvas->RestoreScreenImage(bounds);
}

//////////////////////////////////////////////////////////////////////
//
// ScreenTextBlock
//
ScreenTextBlock::ScreenTextBlock(ui::CaretOwner* caret_owner)
    : caret_(new Caret(caret_owner)) {}

ScreenTextBlock::~ScreenTextBlock() {}

gfx::RectF ScreenTextBlock::HitTestTextPosition(text::Posn offset) const {
  const auto& lines = layout_view_->lines();
  if (offset < lines.front()->text_start() ||
      offset > lines.back()->text_end()) {
    return gfx::RectF();
  }
  for (auto const line : lines) {
    auto const rect = line->HitTestTextPosition(offset);
    if (!rect.empty())
      return RoundBounds(rect);
  }
  return gfx::RectF();
}

void ScreenTextBlock::Paint(gfx::Canvas* canvas,
                            scoped_refptr<LayoutView> layout_view,
                            base::Time now) {
  if (layout_view_ &&
      layout_view_->layout_version() == layout_view->layout_version()) {
    PaintSelectionIfNeeded(canvas, layout_view, now);
    return;
  }
  paint::RootInlineBoxListPainter painter(
      canvas, layout_view->bounds(), layout_view->bgcolor(),
      layout_view->lines(),
      layout_view_ ? layout_view_->lines() : std::vector<RootInlineBox*>{});
  caret_->DidPaint(layout_view->bounds());
  layout_view_ = layout_view;
  if (!painter.Paint()) {
    TRACE_EVENT0("view", "ScreenTextBlock::PaintClean");
    PaintSelection(canvas, now);
    return;
  }

  TRACE_EVENT0("view", "ScreenTextBlock::PaintDirty");
  auto const saved = canvas->SaveScreenImage(layout_view->bounds());
  painter.Finish();
  PaintSelection(canvas, now);
  paint::LayoutViewPainter(canvas).Paint(*layout_view_);
  if (saved)
    return;
  layout_view_ = nullptr;
}

void ScreenTextBlock::PaintSelection(gfx::Canvas* canvas, base::Time now) {
  TRACE_EVENT0("view", "ScreenTextBlock::PaintSelection");
  const auto& lines = layout_view_->lines();
  const auto& selection = layout_view_->selection();
  if (selection.start() >= lines.back()->text_end()) {
    caret_->Hide(canvas);
    return;
  }
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, layout_view_->bounds());
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

void ScreenTextBlock::PaintSelectionIfNeeded(
    gfx::Canvas* canvas,
    scoped_refptr<LayoutView> layout_view,
    base::Time now) {
  const auto& new_selection = layout_view->selection();
  const auto& old_selection = layout_view_->selection();
  layout_view_ = layout_view;
  if (old_selection == new_selection) {
    if (!old_selection.has_focus())
      return;
    caret_->Blink(canvas, now);
    return;
  }
  const auto& bounds = layout_view_->bounds();
  const auto& lines = layout_view_->lines();
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
  UpdateCaret(canvas, now);
}

void ScreenTextBlock::Reset() {
  layout_view_ = nullptr;
  caret_->Reset();
}

void ScreenTextBlock::UpdateCaret(gfx::Canvas* canvas, base::Time now) {
  DCHECK(!caret_->visible());
  const auto& selection = layout_view_->selection();
  if (!selection.has_focus())
    return;
  auto const char_rect = HitTestTextPosition(selection.focus_offset());
  if (char_rect.empty())
    return;
  auto const caret_width = 2;
  gfx::RectF caret_bounds(char_rect.left, char_rect.top,
                          char_rect.left + caret_width, char_rect.bottom);
  caret_->Update(canvas, now, caret_bounds);
}

}  // namespace rendering
}  // namespace views
