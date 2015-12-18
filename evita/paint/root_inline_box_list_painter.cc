// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "evita/paint/root_inline_box_list_painter.h"

#include "base/logging.h"
#include "evita/gfx_base.h"
#include "evita/paint/public/line/inline_box.h"
#include "evita/paint/public/line/root_inline_box.h"
#include "evita/paint/root_inline_box_painter.h"
#include "evita/views/switches.h"

#define DEBUG_DRAW 0
// USE_OVERLAY controls how redraw marker rendered. If USE_OVERLAY is true,
// redraw marker is drawn as overlay rectangle, otherwise, redraw marker
// is rendered at left border.
#define USE_OVERLAY 1

namespace paint {

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

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// RootInlineBoxListPainter
//
RootInlineBoxListPainter::RootInlineBoxListPainter(
    gfx::Canvas* canvas,
    const gfx::RectF& bounds,
    const gfx::ColorF& bgcolor,
    const std::vector<RootInlineBox*>& format_lines,
    const std::vector<RootInlineBox*>& screen_lines)
    : bgcolor_(bgcolor),
      bounds_(bounds),
      canvas_(canvas),
      format_lines_(format_lines),
      screen_lines_(screen_lines) {}

RootInlineBoxListPainter::~RootInlineBoxListPainter() {}

void RootInlineBoxListPainter::Copy(float dst_top,
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
  canvas_->Flush();
}

void RootInlineBoxListPainter::DrawDirtyRect(const gfx::RectF& rect,
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

void RootInlineBoxListPainter::FillBottom(const RootInlineBox* line) const {
  auto const rect = gfx::RectF(gfx::PointF(bounds_.left, line->bottom()),
                               bounds_.bottom_right())
                        .Intersect(bounds_);
  if (rect.empty())
    return;
  gfx::Brush fill_brush(canvas_, bgcolor_);
  canvas_->FillRectangle(fill_brush, rect);
  canvas_->AddDirtyRect(rect);
}

void RootInlineBoxListPainter::FillRight(const RootInlineBox* line) const {
  auto const rect = gfx::RectF(line->origin() + gfx::SizeF(line->width(), 0.0f),
                               gfx::PointF(bounds_.right, line->bottom()))
                        .Intersect(bounds_);
  if (rect.empty())
    return;
  gfx::Brush fill_brush(canvas_, bgcolor_);
  canvas_->FillRectangle(fill_brush, rect);
  canvas_->AddDirtyRect(rect);
}

RootInlineBoxListPainter::FormatLineIterator
RootInlineBoxListPainter::FindFirstMismatch() const {
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

RootInlineBoxListPainter::FormatLineIterator
RootInlineBoxListPainter::FindLastMatch() const {
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

RootInlineBoxListPainter::FormatLineIterator
RootInlineBoxListPainter::FindCopyable(RootInlineBox* format_line) const {
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

void RootInlineBoxListPainter::Finish() {
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

bool RootInlineBoxListPainter::Paint() {
#if DEBUG_DRAW
  DVLOG(0) << "Start painting";
#endif

  if (VLOG_IS_ON(0)) {
    // TextBlock must cover whole screen area.
    auto const last_format_line = format_lines_.back();
    if (!last_format_line->last_box()->is<InlineMarkerBox>())
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
      RootInlineBoxPainter(*format_line).Paint(canvas_);
      FillRight(format_line);
      AddRect(&dirty_rects_, format_line->bounds());
      canvas_->Flush();
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
  DVLOG(0) << "End painting dirty=" << dirty;
#endif
  return dirty;
}

void RootInlineBoxListPainter::RestoreSkipRect(const gfx::RectF& rect) const {
  auto marker_rect = rect;
  marker_rect.left += kMarkerLeftMargin;
  marker_rect.right = marker_rect.left + kMarkerWidth;
  canvas_->FillRectangle(gfx::Brush(canvas_, gfx::ColorF::White), marker_rect);
  canvas_->AddDirtyRect(bounds_.Intersect(marker_rect));
  if (screen_lines_.empty())
    return;
  auto const line_rect = gfx::RectF(gfx::PointF(bounds_.left, rect.top),
                                    gfx::SizeF(bounds_.width(), rect.height()))
                             .Intersect(bounds_);
  if (line_rect.empty())
    return;
  canvas_->RestoreScreenImage(line_rect);
  canvas_->AddDirtyRect(line_rect);
}

RootInlineBoxListPainter::FormatLineIterator RootInlineBoxListPainter::TryCopy(
    const FormatLineIterator& format_current,
    const FormatLineIterator& format_end) const {
  if (screen_lines_.empty())
    return format_current;

  auto const screen_end = screen_lines_.end();
  auto format_runner = format_current;
  while (format_runner != format_end) {
    // TODO(eval1749): Should we search longest match? How?
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

}  // namespace paint
