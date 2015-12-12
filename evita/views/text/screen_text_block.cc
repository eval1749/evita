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
#include "evita/paint/root_inline_box_list_painter.h"
#include "evita/ui/caret.h"
#include "evita/views/text/layout_view.h"
//#include "evita/views/text/inline_box.h"
#include "evita/views/text/render_selection.h"
#include "evita/views/text/root_inline_box.h"

#define DEBUG_DRAW 0
// USE_OVERLAY controls how redraw marker rendered. If USE_OVERLAY is true,
// redraw marker is drawn as overlay rectangle, otherwise, redraw marker
// is rendered at left border.
#define USE_OVERLAY 1

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

inline void FillRect(gfx::Canvas* canvas,
                     const gfx::RectF& rect,
                     gfx::ColorF color) {
  gfx::Brush fill_brush(canvas, color);
  canvas->FillRectangle(fill_brush, rect);
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
    : caret_(new Caret(caret_owner)), dirty_(true), has_screen_bitmap_(false) {}

ScreenTextBlock::~ScreenTextBlock() {}

gfx::RectF ScreenTextBlock::HitTestTextPosition(text::Posn offset) const {
  if (offset < lines_.front()->text_start() ||
      offset > lines_.back()->text_end()) {
    return gfx::RectF();
  }
  for (auto const line : lines_) {
    auto const rect = line->HitTestTextPosition(offset);
    if (!rect.empty())
      return RoundBounds(rect);
  }
  return gfx::RectF();
}

void ScreenTextBlock::Paint(gfx::Canvas* canvas,
                            const LayoutView& layout_view,
                            base::Time now) {
  if (has_screen_bitmap_) {
    DCHECK(canvas->screen_bitmap());
    DCHECK(!lines_.empty());
  } else {
    DCHECK(lines_.empty());
  }

  paint::RootInlineBoxListPainter painter(
      canvas, bounds_, layout_view.bgcolor(), layout_view.lines(), lines_);
  dirty_ = painter.Paint();
  caret_->DidPaint(bounds_);
  if (!dirty_) {
    TRACE_EVENT0("view", "ScreenTextBlock::PaintClean");
    // Contents of lines aren't changed. But, text offset of lines may be
    // changed.
    auto runner = lines_.begin();
    for (const auto& line : layout_view.lines()) {
      if (line->text_start() != (*runner)->text_start()) {
        delete *runner;
        *runner = line->Copy();
      }
      ++runner;
    }
    PaintSelection(canvas, layout_view.selection(), now);
    return;
  }

  TRACE_EVENT0("view", "ScreenTextBlock::PaintDirty");
  Reset();
  has_screen_bitmap_ = canvas->SaveScreenImage(bounds_);
  // Event if we can't get bitmap from render target, screen is up-to-date,
  // but we render all lines next time.
  if (has_screen_bitmap_) {
    // TODO(eval1749): We should use existing RootInlineBox's in
    // ScreenTextBlock.
    for (const auto& line : layout_view.lines()) {
      lines_.push_back(line->Copy());
    }
  }
  painter.Finish();
  PaintSelection(canvas, layout_view.selection(), now);
  dirty_ = false;
}

void ScreenTextBlock::PaintSelection(gfx::Canvas* canvas,
                                     const TextSelection& selection,
                                     base::Time now) {
  TRACE_EVENT0("view", "ScreenTextBlock::PaintSelection");
  selection_ = selection;
  if (selection_.start() >= lines_.back()->text_end()) {
    caret_->Hide(canvas);
    return;
  }
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, bounds_);
  if (selection.is_range() && selection_.end() > lines_.front()->text_start()) {
    gfx::Brush fill_brush(canvas, selection_.color());
    for (auto line : lines_) {
      if (selection_.end() <= line->text_start())
        break;
      auto const rect = line->CalculateSelectionRect(selection);
      if (rect.empty())
        continue;
      canvas->FillRectangle(fill_brush, rect);
    }
  }
  UpdateCaret(canvas, now);
}

void ScreenTextBlock::PaintSelectionIfNeeded(gfx::Canvas* canvas,
                                             const TextSelection& new_selection,
                                             base::Time now) {
  if (selection_ == new_selection) {
    if (!selection_.has_focus())
      return;
    caret_->Blink(canvas, now);
    return;
  }
  auto new_selection_rects =
      CalculateSelectionRects(lines_, new_selection, bounds_);
  auto old_selection_rects =
      CalculateSelectionRects(lines_, selection_, bounds_);

  gfx::Canvas::DrawingScope drawing_scope(canvas);
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, bounds_);
  for (const auto& old_rect : old_selection_rects) {
    if (new_selection_rects.find(old_rect) != new_selection_rects.end())
      continue;
    canvas->AddDirtyRect(old_rect);
    canvas->RestoreScreenImage(old_rect);
    caret_->DidPaint(old_rect);
  }

  if (selection_.color() != new_selection.color())
    old_selection_rects.clear();
  if (!new_selection_rects.empty()) {
    gfx::Brush fill_brush(canvas, new_selection.color());
    for (const auto& new_rect : new_selection_rects) {
      if (old_selection_rects.find(new_rect) != old_selection_rects.end())
        continue;
      canvas->AddDirtyRect(new_rect);
      canvas->RestoreScreenImage(new_rect);
      canvas->FillRectangle(fill_brush, new_rect);
      caret_->DidPaint(new_rect);
    }
  }

  selection_ = new_selection;
  caret_->Hide(canvas);
  UpdateCaret(canvas, now);
}

void ScreenTextBlock::Reset() {
  dirty_ = true;
  for (auto line : lines_) {
    delete line;
  }
  lines_.clear();
  has_screen_bitmap_ = false;
  selection_ = TextSelection();
  caret_->Reset();
}

void ScreenTextBlock::SetBounds(const gfx::RectF& new_bounds) {
  Reset();
  bounds_ = new_bounds;
}

void ScreenTextBlock::UpdateCaret(gfx::Canvas* canvas, base::Time now) {
  DCHECK(!caret_->visible());
  if (!selection_.has_focus())
    return;
  auto const char_rect = HitTestTextPosition(selection_.focus_offset());
  if (char_rect.empty())
    return;
  auto const caret_width = 2;
  gfx::RectF caret_bounds(char_rect.left, char_rect.top,
                          char_rect.left + caret_width, char_rect.bottom);
  caret_->Update(canvas, now, caret_bounds);
}

}  // namespace rendering
}  // namespace views
