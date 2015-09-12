// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <list>
#include <unordered_set>

#include "evita/views/text/screen_text_block.h"

#include "base/logging.h"
#include "base/time/time.h"
#include "evita/gfx/bitmap.h"
#include "evita/ui/caret.h"
#include "evita/views/switches.h"
#include "evita/views/text/render_cell.h"
#include "evita/views/text/render_selection.h"
#include "evita/views/text/render_text_block.h"
#include "evita/views/text/render_text_line.h"

#define DEBUG_DRAW 0
// USE_OVERLAY controls how redraw marker rendered. If USE_OVERLAY is true,
// redraw marker is drawn as overlay rectangle, otherwise, redraw marker
// is rendered at left border.
#define USE_OVERLAY 1

namespace views {
namespace rendering {

namespace {

const auto kMarkerLeftMargin = 2.0f;
const auto kMarkerWidth = 4.0f;

void AddRect(std::vector<gfx::RectF>* rects, const gfx::RectF& rect) {
  if (rects->empty()) {
    rects->push_back(rect);
    return;
  }

  auto& last = rects->back();
  if (last.bottom == rect.top) {
    last.right = std::max(last.right, rect.right);
    last.bottom = rect.bottom;
    return;
  }

  if (last.top == rect.bottom) {
    last.right = std::max(last.right, rect.right);
    last.top = rect.top;
    return;
  }

  rects->push_back(rect);
}

inline gfx::RectF RoundBounds(const gfx::RectF& bounds) {
  return gfx::RectF(::floor(bounds.left), ::floor(bounds.top),
                    ::floor(bounds.right), ::floor(bounds.bottom));
}

std::unordered_set<gfx::RectF> CalculateSelectionRects(
    const std::vector<TextLine*>& lines,
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

using FormatLineIterator = std::list<TextLine*>::const_iterator;

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
// ScreenTextBlock::RenderContext
//
class ScreenTextBlock::RenderContext final {
 public:
  RenderContext(const ScreenTextBlock* screen_text_block,
                gfx::Canvas* canvas,
                const TextBlock* format_text_block);
  ~RenderContext() = default;

  void Finish();
  bool Render();
  FormatLineIterator TryCopy(const FormatLineIterator& format_line_start,
                             const FormatLineIterator& format_line_end) const;

 private:
  void Copy(float dst_top, float dst_bottom, float src_top) const;
  void DrawDirtyRect(const gfx::RectF& rect,
                     float red,
                     float green,
                     float blue) const;
  void FillBottom(const TextLine* line) const;
  void FillRight(const TextLine* line) const;
  FormatLineIterator FindFirstMismatch() const;
  FormatLineIterator FindLastMatch() const;
  std::vector<TextLine*>::const_iterator FindCopyable(TextLine* line) const;
  void RestoreSkipRect(const gfx::RectF& rect) const;

  const gfx::ColorF bgcolor_;
  const gfx::RectF bounds_;
  gfx::Canvas* canvas_;
  mutable std::vector<gfx::RectF> copy_rects_;
  mutable std::vector<gfx::RectF> dirty_rects_;
  const std::list<TextLine*>& format_lines_;
  const std::vector<TextLine*>& screen_lines_;
  const ScreenTextBlock* screen_text_block_;
  mutable std::vector<gfx::RectF> skip_rects_;

  DISALLOW_COPY_AND_ASSIGN(RenderContext);
};

ScreenTextBlock::RenderContext::RenderContext(
    const ScreenTextBlock* screen_text_block,
    gfx::Canvas* canvas,
    const TextBlock* format_text_block)
    : bgcolor_(format_text_block->default_style().bgcolor()),
      bounds_(screen_text_block->bounds_),
      canvas_(canvas),
      format_lines_(format_text_block->lines()),
      screen_text_block_(screen_text_block),
      screen_lines_(screen_text_block->lines_) {}

void ScreenTextBlock::RenderContext::Copy(float dst_top,
                                          float dst_bottom,
                                          float src_top) const {
  auto const height = dst_bottom - dst_top;
  DCHECK_GT(height, 0.0f);
  DCHECK_LE(src_top + height, bounds_.bottom);

  gfx::RectF dst_rect(bounds_.left, dst_top, bounds_.right, dst_top + height);
  gfx::RectF src_rect(bounds_.left, src_top, bounds_.right, src_top + height);
  DCHECK_EQ(dst_rect.size(), src_rect.size());
#if DEBUG_DRAW
  DVLOG(0) << "Copy to " << dst_rect << " from " << src_rect.origin();
#endif
  canvas_->DrawBitmap(*canvas_->screen_bitmap(), dst_rect, src_rect);
}

void ScreenTextBlock::RenderContext::DrawDirtyRect(const gfx::RectF& rect,
                                                   float red,
                                                   float green,
                                                   float blue) const {
  RestoreSkipRect(rect);
  if (views::switches::text_window_display_paint) {
#if USE_OVERLAY
    canvas_->FillRectangle(gfx::Brush(canvas_, red, green, blue, 0.1f), rect);
    canvas_->DrawRectangle(gfx::Brush(canvas_, red, green, blue, 0.5f), rect,
                           0.5f);
#else
    auto marker_rect = rect;
    marker_rect.left += kMarkerLeftMargin;
    marker_rect.right = marker_rect.left + kMarkerWidth;
    canvas_->FillRectangle(gfx::Brush(canvas_, red, green, blue), marker_rect);
#endif
  }
}

void ScreenTextBlock::RenderContext::FillBottom(const TextLine* line) const {
  auto const rect = gfx::RectF(gfx::PointF(bounds_.left, line->bottom()),
                               bounds_.bottom_right())
                        .Intersect(bounds_);
  if (rect.empty())
    return;
  gfx::Brush fill_brush(canvas_, bgcolor_);
  canvas_->FillRectangle(fill_brush, rect);
  canvas_->AddDirtyRect(rect);
}

void ScreenTextBlock::RenderContext::FillRight(const TextLine* line) const {
  auto const rect =
      gfx::RectF(line->origin() + gfx::SizeF(line->GetWidth(), 0.0f),
                 gfx::PointF(bounds_.right, line->bottom()))
          .Intersect(bounds_);
  if (rect.empty())
    return;
  gfx::Brush fill_brush(canvas_, bgcolor_);
  canvas_->FillRectangle(fill_brush, rect);
  canvas_->AddDirtyRect(rect);
}

FormatLineIterator ScreenTextBlock::RenderContext::FindFirstMismatch() const {
  auto screen_line_runner = screen_lines_.cbegin();
  for (auto format_line_runner = format_lines_.cbegin();
       format_line_runner != format_lines_.cend(); ++format_line_runner) {
    if (screen_line_runner == screen_lines_.cend())
      return format_line_runner;
    auto const format_line = (*format_line_runner);
    auto const screen_line = (*screen_line_runner);
    if (format_line->bounds() != screen_line->bounds() ||
        !format_line->Equal(screen_line)) {
      return format_line_runner;
    }
    AddRect(&skip_rects_, format_line->bounds());
    ++screen_line_runner;
  }
  return format_lines_.cend();
}

FormatLineIterator ScreenTextBlock::RenderContext::FindLastMatch() const {
  auto screen_line_runner = screen_lines_.crbegin();
  auto format_last_match = format_lines_.crbegin();
  for (auto format_line_runner = format_lines_.crbegin();
       format_line_runner != format_lines_.crend(); ++format_line_runner) {
    auto const format_line = (*format_line_runner);
    if (format_line->top() >= bounds_.bottom)
      continue;
    if (screen_line_runner == screen_lines_.crend())
      break;
    auto const screen_line = (*screen_line_runner);
    if (format_line->bounds() != screen_line->bounds() ||
        !format_line->Equal(screen_line)) {
      break;
    }
    AddRect(&skip_rects_, format_line->bounds());
    format_last_match = format_line_runner;
    ++screen_line_runner;
  }

  return format_last_match == format_lines_.crbegin()
             ? format_lines_.end()
             : std::find(format_lines_.cbegin(), format_lines_.cend(),
                         *format_last_match);
}

std::vector<TextLine*>::const_iterator
ScreenTextBlock::RenderContext::FindCopyable(TextLine* format_line) const {
  for (auto runner = screen_lines_.begin(); runner != screen_lines_.end();
       ++runner) {
    auto const screen_line = *runner;
    if (screen_line->Equal(format_line) &&
        (format_line->bounds().top == screen_line->bounds().top ||
         screen_line->bounds().bottom <= bounds_.bottom)) {
      return runner;
    }
  }
  return screen_lines_.end();
}

void ScreenTextBlock::RenderContext::Finish() {
  // Draw dirty rectangles for debugging.
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas_, bounds_);
  for (auto rect : copy_rects_) {
#if DEBUG_DRAW
    DVLOG(0) << "copy " << rect;
#endif
    DrawDirtyRect(rect, 58.0f / 255, 128.0f / 255, 247.0f / 255);
    canvas_->AddDirtyRect(bounds_.Intersect(rect));
  }
  for (auto rect : dirty_rects_) {
#if DEBUG_DRAW
    DVLOG(0) << "dirty " << rect;
#endif
    DrawDirtyRect(rect, 219.0f / 255, 68.0f / 255, 55.0f / 255);
    canvas_->AddDirtyRect(bounds_.Intersect(rect));
  }
}

bool ScreenTextBlock::RenderContext::Render() {
#if DEBUG_DRAW
  DVLOG(0) << "Start rendering";
#endif

  if (VLOG_IS_ON(0)) {
    // TextBlock must cover whole screen area.
    auto const last_format_line = format_lines_.back();
    if (!last_format_line->cells().back()->is<MarkerCell>())
      DCHECK_GE(last_format_line->bounds().bottom, bounds_.bottom);
  }

  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas_, bounds_);

  auto const dirty_line_start = FindFirstMismatch();
  if (dirty_line_start != format_lines_.end()) {
    auto const clean_line_start = FindLastMatch();
#if DEBUG_DRAW
    DVLOG(0) << "dirty " << (*dirty_line_start)->bounds().top << ","
             << (clean_line_start == format_lines_.end()
                     ? bounds_.bottom
                     : (*clean_line_start)->bounds().top);
#endif
    for (auto dirty_line_runner = dirty_line_start;
         dirty_line_runner != clean_line_start; ++dirty_line_runner) {
      dirty_line_runner = TryCopy(dirty_line_runner, clean_line_start);
      if (dirty_line_runner == clean_line_start)
        break;
      auto const format_line = *dirty_line_runner;
      format_line->Render(canvas_);
      FillRight(format_line);
      AddRect(&dirty_rects_, format_line->bounds());
    }
  }

  // Erase dirty rectangle markers.
  for (auto rect : skip_rects_) {
#if DEBUG_DRAW
    DVLOG(0) << "skip " << rect;
#endif
    RestoreSkipRect(rect);
  }
  FillBottom(format_lines_.back());

  auto const dirty = !copy_rects_.empty() || !dirty_rects_.empty();
#if DEBUG_DRAW
  DVLOG(0) << "End rendering dirty=" << dirty;
#endif
  return dirty;
}

void ScreenTextBlock::RenderContext::RestoreSkipRect(
    const gfx::RectF& rect) const {
  auto marker_rect = rect;
  marker_rect.left += kMarkerLeftMargin;
  marker_rect.right = marker_rect.left + kMarkerWidth;
  canvas_->FillRectangle(gfx::Brush(canvas_, gfx::ColorF::White), marker_rect);
  canvas_->AddDirtyRect(bounds_.Intersect(marker_rect));
  if (!screen_text_block_->has_screen_bitmap_)
    return;
  auto const line_rect = gfx::RectF(gfx::PointF(bounds_.left, rect.top),
                                    gfx::SizeF(bounds_.width(), rect.height()))
                             .Intersect(bounds_);
  if (line_rect.empty())
    return;
  canvas_->RestoreScreenImage(line_rect);
  canvas_->AddDirtyRect(line_rect);
}

FormatLineIterator ScreenTextBlock::RenderContext::TryCopy(
    const FormatLineIterator& format_current,
    const FormatLineIterator& format_end) const {
  if (!screen_text_block_->has_screen_bitmap_)
    return format_current;

  auto const screen_end = screen_lines_.end();
  auto format_runner = format_current;
  while (format_runner != format_end) {
    // TODO(yosi) Should we search longest match? How?
    auto const format_start = format_runner;
    auto const screen_start = FindCopyable(*format_start);
    if (screen_start == screen_lines_.end())
      return format_runner;

    auto const dst_top = (*format_start)->top();
    auto const src_top = (*screen_start)->top();

    auto const skip = dst_top == src_top;
    if (skip)
      AddRect(&skip_rects_, (*format_start)->bounds());
    else
      AddRect(&copy_rects_, (*format_start)->bounds());

    auto dst_bottom = (*format_start)->bottom();
    ++format_runner;
    auto screen_runner = screen_start;
    ++screen_runner;

    while (format_runner != format_end && screen_runner != screen_end) {
      auto const format_line = *format_runner;
      auto const screen_line = *screen_runner;
      if (screen_line->bounds().bottom > bounds_.bottom ||
          !format_line->Equal(screen_line))
        break;
      if (skip)
        AddRect(&skip_rects_, format_line->bounds());
      else
        AddRect(&copy_rects_, format_line->bounds());
      dst_bottom = format_line->bottom();
      ++format_runner;
      ++screen_runner;
    }

    if (!skip)
      Copy(dst_top, dst_bottom, src_top);
  }
  return format_runner;
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

void ScreenTextBlock::Render(gfx::Canvas* canvas,
                             const TextBlock* text_block,
                             const TextSelection& selection,
                             base::Time now) {
  DCHECK(!text_block->dirty());
  DCHECK(!has_screen_bitmap_ || canvas->screen_bitmap());
  RenderContext render_context(this, canvas, text_block);
  dirty_ = render_context.Render();
  caret_->DidPaint(bounds_);
  if (!dirty_) {
    // Contents of lines aren't changed. But, text offset of lines may be
    // changed.
    auto runner = lines_.begin();
    for (auto line : text_block->lines()) {
      if (line->text_start() != (*runner)->text_start()) {
        delete *runner;
        *runner = line->Copy();
      }
      ++runner;
    }
    RenderSelection(canvas, selection, now);
    return;
  }

  Reset();
  has_screen_bitmap_ = canvas->SaveScreenImage(bounds_);
  // Event if we can't get bitmap from render target, screen is up-to-date,
  // but we render all lines next time.
  if (has_screen_bitmap_) {
    // TODO(yosi) We should use existing TextLine's in ScreenTextBlock.
    for (auto line : text_block->lines()) {
      lines_.push_back(line->Copy());
    }
  }
  render_context.Finish();
  RenderSelection(canvas, selection, now);
  dirty_ = false;
}

void ScreenTextBlock::RenderSelection(gfx::Canvas* canvas,
                                      const TextSelection& selection,
                                      base::Time now) {
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

void ScreenTextBlock::RenderSelectionIfNeeded(
    gfx::Canvas* canvas,
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
