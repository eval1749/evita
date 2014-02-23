// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/text/screen_text_block.h"

#include "base/logging.h"
#include "evita/views/text/render_selection.h"
#include "evita/views/text/render_text_block.h"
#include "evita/views/text/render_text_line.h"

#define DEBUG_DRAW 0

namespace views {
namespace rendering {

namespace {

std::unique_ptr<gfx::Bitmap> CopyFromRenderTarget(const gfx::Graphics* gfx,
                                                  const gfx::RectF rect) {
  auto bitmap = std::make_unique<gfx::Bitmap>(*gfx);
  auto const rect_u = gfx::RectU(static_cast<uint32_t>(rect.left),
                                 static_cast<uint32_t>(rect.top),
                                 static_cast<uint32_t>(rect.right),
                                 static_cast<uint32_t>(rect.bottom));
  DCHECK_EQ(static_cast<float>(rect_u.width()), rect.width());
  DCHECK_EQ(static_cast<float>(rect_u.height()), rect.height());
  #if DEBUG_DRAW
    DVLOG(0) << "CopyFromRenderTarget " << rect;
  #endif
  auto const hr = (*bitmap)->CopyFromRenderTarget(nullptr, *gfx, &rect_u);
  if (FAILED(hr)) {
     return std::unique_ptr<gfx::Bitmap>();
  }
  return std::move(bitmap);
}

inline void FillRect(const gfx::Graphics& gfx, const gfx::RectF& rect,
                     gfx::ColorF color) {
  gfx::Brush fill_brush(gfx, color);
  gfx.FillRectangle(fill_brush, rect);
}

} // namespace

typedef std::list<TextLine*>::const_iterator LineIterator;

//////////////////////////////////////////////////////////////////////
//
// ScreenTextBlock::RenderContext
//
class ScreenTextBlock::RenderContext {
  private: const gfx::ColorF bgcolor_;
  private: mutable std::vector<gfx::RectF> copy_rects_;
  private: const gfx::Graphics* gfx_;
  private: const gfx::RectF rect_;
  private: std::vector<TextLine*> lines_;
  private: const ScreenTextBlock* screen_text_block_;

  public: RenderContext(const ScreenTextBlock* screen_text_block,
                        gfx::ColorF bgcolor);
  public: ~RenderContext() = default;

  public: const std::vector<gfx::RectF>& copy_rects() const {
    return copy_rects_;
  }

  private: void Copy(float dst_top, float dst_bottom, float src_top) const;
  public: void FillBottom(const TextLine* line) const;
  public: void FillRight(const TextLine* line) const;
  private: std::vector<TextLine*>::const_iterator FindSameLine(
      const TextLine* line) const;
  public: LineIterator TryCopy(const LineIterator& new_start,
                               const LineIterator& new_end) const;

  DISALLOW_COPY_AND_ASSIGN(RenderContext);
};

ScreenTextBlock::RenderContext::RenderContext(
    const ScreenTextBlock* screen_text_block, gfx::ColorF bgcolor)
    : bgcolor_(bgcolor), screen_text_block_(screen_text_block),
      gfx_(screen_text_block->gfx_), rect_(screen_text_block->rect_) {
  for (auto line : screen_text_block->lines_) {
    if (line->bottom() > rect_.bottom)
      break;
    lines_.push_back(line);
  }
}

void ScreenTextBlock::RenderContext::Copy(float dst_top, float dst_bottom,
                                          float src_top) const {
  auto const src_bottom = src_top + dst_bottom - dst_top;

  auto const height = std::min(
      std::min(rect_.bottom, dst_bottom) - dst_top,
      std::min(rect_.bottom, src_bottom) - src_top);

  gfx::RectF dst_rect(rect_.left, dst_top, rect_.right, dst_top + height);
  gfx::RectF src_rect(0.0f, src_top - lines_.front()->top(),
                      rect_.width(), src_top + height - lines_.front()->top());
  DCHECK_EQ(dst_rect.size(), src_rect.size());
  #if DEBUG_DRAW
    DVLOG(0) << "Copy to " << dst_rect << " from " << src_rect;
  #endif
  (*gfx_)->DrawBitmap(*screen_text_block_->bitmap_, dst_rect, 1.0f,
                      D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
                      src_rect);
  copy_rects_.push_back(dst_rect);
}

void ScreenTextBlock::RenderContext::FillBottom(const TextLine* line) const {
  gfx::RectF rect(rect_);;
  rect.top = line->bottom();
  if (rect.empty())
    return;
  FillRect(*gfx_, rect, bgcolor_);
}

void ScreenTextBlock::RenderContext::FillRight(const TextLine* line) const {
  gfx::RectF rect(line->rect());
  rect.left  = rect_.left + line->GetWidth();
  rect.right = rect_.right;
  if (rect.empty())
    return;
  FillRect(*gfx_, rect, bgcolor_);
}

std::vector<TextLine*>::const_iterator
    ScreenTextBlock::RenderContext::FindSameLine(const TextLine* line) const {
  for (auto runner = lines_.begin(); runner != lines_.end(); ++runner) {
    if ((*runner)->Equal(line))
      return runner;
  }
  return lines_.end();
}

LineIterator ScreenTextBlock::RenderContext::TryCopy(
    const LineIterator& new_start, const LineIterator& new_end) const {
  if (!screen_text_block_->bitmap_)
    return new_start;

  auto present_start = FindSameLine(*new_start);
  if (present_start == lines_.end())
    return new_start;

  auto const present_end = lines_.end();

  auto new_last = new_start;
  auto new_runner = new_start;
  ++new_runner;
  auto present_runner = present_start;
  ++present_runner;
  while (new_runner != new_end && present_runner != present_end) {
    if ((*present_runner)->bottom() > rect_.bottom ||
        !(*new_runner)->Equal(*present_runner)) {
      break;
    }
    new_last = new_runner;
    ++new_runner;
    ++present_runner;
  }

  if ((*new_start)->top() != (*present_start)->top()) {
    Copy((*new_start)->top(), (*new_last)->bottom(),
         (*present_start)->top());
    ++new_last;
  }
  return new_last;
}

//////////////////////////////////////////////////////////////////////
//
// ScreenTextBlock
//
ScreenTextBlock::ScreenTextBlock()
    : dirty_(true), gfx_(nullptr) {
}

ScreenTextBlock::~ScreenTextBlock() {
}

void ScreenTextBlock::Render(const TextBlock* text_block, gfx::ColorF bgcolor) {
  auto const format_line_end = text_block->lines().end();
  auto format_line_runner = text_block->lines().begin();
  auto const screen_line_end = lines_.end();
  auto screen_line_runner = lines_.begin();
  while (format_line_runner != format_line_end &&
         screen_line_runner != screen_line_end) {
    if ((*format_line_runner)->rect() != (*screen_line_runner)->rect() ||
        !(*format_line_runner)->Equal(*screen_line_runner)) {
      break;
    }
    ++format_line_runner;
    ++screen_line_runner;
  }

  RenderContext render_context(this, bgcolor);
  std::vector<gfx::RectF> dirty_rects;
  {
    // Note: ID2D1Bitmap::CopyFromRenderTarget should be called without
    // clipping.
    gfx::Graphics::AxisAlignedClipScope clip_scope(*gfx_, rect_);
    while (format_line_runner != format_line_end) {
      format_line_runner = render_context.TryCopy(format_line_runner,
                                                  format_line_end);
      if (format_line_runner == format_line_end)
        break;
      auto const format_line = *format_line_runner;
      format_line->Render(*gfx_);
      render_context.FillRight(format_line);
      if (dirty_rects.empty() ||
          dirty_rects.back().bottom != format_line->top()) {
        dirty_rects.push_back(format_line->rect());
      } else {
        auto& last = dirty_rects.back();
        last.right = std::max(last.right, format_line->right());
        last.bottom = format_line->bottom();
      }
      ++format_line_runner;
      dirty_ = true;
    }
  }

  if (!dirty_)
    return;

  Reset();
  // TODO(yosi) We should use existing TextLine's in ScreenTextBlock.
  for (auto line : text_block->lines()) {
    lines_.push_back(line->Copy());
  }
  render_context.FillBottom(lines_.back());

  bitmap_ = CopyFromRenderTarget(gfx_, gfx::RectF(
      gfx::PointF(rect_.left, lines_.front()->rect().top),
      gfx::SizeF(rect_.width(), lines_.back()->rect().bottom)));

  auto const bitmap_origin = lines_.front()->rect().left_top();
  for (auto rect : render_context.copy_rects()) {
    gfx_->DrawBitmap(*bitmap_,
                     gfx::RectF(rect_.left, rect.top, rect_.right, rect.bottom),
                     gfx::RectF(0.0f, rect.top - bitmap_origin.y,
                                rect_.width(), rect.bottom - bitmap_origin.y));
    gfx_->FillRectangle(gfx::Brush(*gfx_, 0.0f, 1.0f, 1.0f, 0.05f), rect);
    gfx_->DrawRectangle(gfx::Brush(*gfx_, 0.0f, 1.0f, 1.0f, 0.1f), rect, 0.5f);
  }
  for (auto rect : dirty_rects) {
    gfx_->DrawBitmap(*bitmap_,
                     gfx::RectF(rect_.left, rect.top, rect_.right, rect.bottom),
                     gfx::RectF(0.0f, rect.top - bitmap_origin.y,
                                rect_.width(), rect.bottom - bitmap_origin.y));
    gfx_->FillRectangle(gfx::Brush(*gfx_, 1.0f, 0.0f, 1.0f, 0.05f), rect);
    gfx_->DrawRectangle(gfx::Brush(*gfx_, 1.0f, 0.0f, 1.0f, 0.1f), rect, 0.5f);
  }

  dirty_ = false;
}

void ScreenTextBlock::Reset() {
  dirty_ = true;
  for (auto line : lines_) {
    delete line;
  }
  lines_.clear();
}

void ScreenTextBlock::SetGraphics(const gfx::Graphics* gfx) {
  Reset();
  gfx_ = gfx;
  bitmap_.reset();
}

void ScreenTextBlock::SetRect(const gfx::RectF& rect) {
  Reset();
  rect_ = rect;
  bitmap_.reset();
}

// gfx::Graphics::Observer
void ScreenTextBlock::ShouldDiscardResources() {
  bitmap_.reset();
}

} // namespace rendering
} // namespace views
