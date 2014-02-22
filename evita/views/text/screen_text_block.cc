// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/text/screen_text_block.h"

#include "base/logging.h"
#include "evita/views/text/render_text_block.h"
#include "evita/views/text/render_text_line.h"

namespace views {
namespace rendering {

namespace {

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
  private: const ScreenTextBlock* destination_;
  private: const gfx::Graphics* gfx_;
  private: const gfx::RectF rect_;
  private: std::vector<TextLine*> lines_;
  private: const std::unique_ptr<gfx::Bitmap> screen_bitmap_;

  public: RenderContext(const ScreenTextBlock* screen_text_block,
                        gfx::ColorF bgcolor);
  public: ~RenderContext() = default;

  private: void Copy(float dst_top, float dst_bottom, float src_top) const;
  private: static std::unique_ptr<gfx::Bitmap> CreateBitmap(
      const gfx::Graphics* gfx);
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
    : bgcolor_(bgcolor), destination_(screen_text_block),
      gfx_(screen_text_block->gfx_), rect_(screen_text_block->rect_),
      screen_bitmap_(CreateBitmap(gfx_)) {
  for (auto line : screen_text_block->lines_) {
    if (line->bottom() > rect_.bottom)
      break;
    lines_.push_back(line);
  }

  // Note: LineCopier uses ID2D1Bitmap::CopyFromRenderTarget. It should be
  // called without clipping.
  gfx::Graphics::AxisAlignedClipScope clip_scope(*gfx_, rect_);
}

void ScreenTextBlock::RenderContext::Copy(float dst_top, float dst_bottom,
                                          float src_top) const {
  auto const src_bottom = src_top + dst_bottom - dst_top;

  auto const height = std::min(
      std::min(rect_.bottom, dst_bottom) - dst_top,
      std::min(rect_.bottom, src_bottom) - src_top);

  auto const right = rect_.right;
  gfx::RectF dst_rect(0.0f, dst_top, right, dst_top + height);
  gfx::RectF src_rect(0.0f, src_top, right, src_top + height);
  DCHECK_EQ(dst_rect.size(), src_rect.size());

  auto const opacity = 1.0f;
  (*gfx_)->DrawBitmap(*screen_bitmap_, dst_rect, opacity,
                      D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
                      src_rect);
}

std::unique_ptr<gfx::Bitmap> ScreenTextBlock::RenderContext::CreateBitmap(
      const gfx::Graphics* gfx) {
  gfx::RectU screen_rect((*gfx)->GetPixelSize());
  auto bitmap = std::make_unique<gfx::Bitmap>(*gfx);
  auto hr = (*bitmap)->CopyFromRenderTarget(nullptr, *gfx, &screen_rect);
  if (FAILED(hr)) {
    DVLOG(0) << "CopyFromRenderTarget: hr=0x" << std::hex << hr;
     return std::unique_ptr<gfx::Bitmap>();
  }
  return std::move(bitmap);
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
  if (!screen_bitmap_)
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

void ScreenTextBlock::Render(const TextBlock* text_block,
                             gfx::ColorF bgcolor) {
  RenderContext render_context(this, bgcolor);
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

  while (format_line_runner != format_line_end) {
    format_line_runner = render_context.TryCopy(format_line_runner,
                                                format_line_end);
    if (format_line_runner == format_line_end)
      break;
    auto const format_line = *format_line_runner;
    format_line->Render(*gfx_);
    render_context.FillRight(format_line);
    ++format_line_runner;
  }

  // TODO(yosi) We should use existing TextLine's in ScreenTextBlock.
  Reset();
  for (auto line : text_block->lines()) {
    lines_.push_back(line->Copy());
  }

  render_context.FillBottom(lines_.back());

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
}

void ScreenTextBlock::SetRect(const gfx::RectF& rect) {
  Reset();
  rect_ = rect;
}

} // namespace rendering
} // namespace views
