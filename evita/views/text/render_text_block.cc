// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/text/render_text_block.h"

#include <limits>
#include <map>
#include <vector>

#include "evita/dom/lock.h"
#include "evita/editor/dom_lock.h"
#include "evita/text/buffer.h"
#include "evita/views/text/render_cell.h"
#include "evita/views/text/render_style.h"
#include "evita/views/text/render_text_line.h"
#include "evita/views/text/text_formatter.h"

namespace views {
namespace rendering {

//////////////////////////////////////////////////////////////////////
//
// TextBlock::TextLineCache
//
class TextBlock::TextLineCache {
  private: gfx::RectF bounds_;
  private: const text::Buffer* const buffer_;
  private: text::Posn dirty_start_;
  private: std::map<text::Posn, TextLine*> lines_;
  private: float zoom_;

  public: TextLineCache(const text::Buffer* buffer);
  public: ~TextLineCache();

  public: void DidChangeBuffer(text::Posn offset);
  public: TextLine* FindLine(text::Posn text_offset) const;
  public: void Invalidate(const gfx::RectF& bounds, float zoom);
  private: bool IsAfterNewline(const TextLine* text_line) const;
  private: bool IsEndWithNewline(const TextLine* text_line) const;
  public: void Register(TextLine* line);
  private: void RemoveDirtyLines();
  private: void RemoveAllLines();

  DISALLOW_COPY_AND_ASSIGN(TextLineCache);
};

TextBlock::TextLineCache::TextLineCache(const text::Buffer* buffer)
    : buffer_(buffer), dirty_start_(std::numeric_limits<text::Posn>::max()),
      zoom_(0.0f) {
}

TextBlock::TextLineCache::~TextLineCache() {
  for (auto& entry : lines_) {
    delete entry.second;
  }
}

void TextBlock::TextLineCache::DidChangeBuffer(text::Posn offset) {
  ASSERT_DOM_LOCKED();
  dirty_start_ = std::min(dirty_start_, offset);
}

TextLine* TextBlock::TextLineCache::FindLine(text::Posn text_offset) const {
  UI_ASSERT_DOM_LOCKED();
  const auto it = lines_.find(text_offset);
  if (it == lines_.end())
    return nullptr;
  auto const line = it->second;
  DCHECK_EQ(line->GetStart(), text_offset);
  return line;
}

void TextBlock::TextLineCache::Invalidate(const gfx::RectF& new_bounds,
                                          float new_zoom) {
  UI_ASSERT_DOM_LOCKED();
  if (zoom_ != new_zoom || !dirty_start_) {
    RemoveAllLines();
    bounds_ = new_bounds;
    dirty_start_ = std::numeric_limits<text::Posn>::max();
    zoom_ = new_zoom;
    return;
  }

  RemoveDirtyLines();
  if (bounds_ == new_bounds)
    return;

  if (!lines_.empty() && bounds_.width() != new_bounds.width()){
    std::vector<text::Posn> dirty_offsets;
    for (auto it : lines_) {
      auto const line = it.second;
      if (line->right() > new_bounds.right || !IsAfterNewline(line) ||
          !IsEndWithNewline(line)) {
        dirty_offsets.push_back(line->GetStart());
      }
    }
    for (auto offset : dirty_offsets) {
      const auto it = lines_.find(offset);
      DCHECK(it != lines_.end());
      delete it->second;
      lines_.erase(it);
    }
  }
  bounds_ = new_bounds;
}

bool TextBlock::TextLineCache::IsAfterNewline(const TextLine* text_line) const {
  auto const start = text_line->GetStart();
  return !start || buffer_->GetCharAt(start - 1) == '\n';
}

bool TextBlock::TextLineCache::IsEndWithNewline(
    const TextLine* text_line) const {
  auto const end = text_line->GetEnd();
  return end >= buffer_->GetEnd() || buffer_->GetCharAt(end) == '\n';
}

void TextBlock::TextLineCache::Register(TextLine* line) {
  UI_ASSERT_DOM_LOCKED();
  DCHECK_GE(line->GetEnd(), line->GetStart());
  lines_[line->GetStart()] = line;
}

void TextBlock::TextLineCache::RemoveDirtyLines() {
  auto const dirty_start = dirty_start_;
  dirty_start_ = std::numeric_limits<text::Posn>::max();
  if (lines_.empty() || dirty_start >= lines_.rbegin()->second->GetEnd())
    return;
  auto it = lines_.lower_bound(dirty_start);
  if (it == lines_.end())
    it = lines_.find(lines_.rbegin()->first);
  while (it != lines_.begin()) {
    --it;
    if (it->second->GetEnd() <= dirty_start) {
      if (!IsEndWithNewline(it->second))
        ++it;
      break;
    }
  }

  if (it == lines_.begin()) {
    RemoveAllLines();
    return;
  }

  std::vector<text::Posn> dirty_offsets;
  while (it != lines_.end()) {
    dirty_offsets.push_back(it->first);
    ++it;
  }
  for (auto offset : dirty_offsets) {
    const auto it = lines_.find(offset);
    DCHECK(it != lines_.end());
    delete it->second;
    lines_.erase(it);
  }
}

void TextBlock::TextLineCache::RemoveAllLines() {
  for (auto entry : lines_) {
    delete entry.second;
  }
  lines_.clear();
}

//////////////////////////////////////////////////////////////////////
//
// TextBlock
//
TextBlock::TextBlock(text::Buffer* text_buffer)
    : dirty_(true), dirty_line_point_(true), format_counter_(0),
      lines_height_(0), text_buffer_(text_buffer),
      text_line_cache_(new TextLineCache(text_buffer)), view_start_(0),
      zoom_(1.0f) {
  text_buffer->AddObserver(this);
}

TextBlock::~TextBlock() {
  text_buffer_->RemoveObserver(this);
}

void TextBlock::Append(TextLine* line) {
  UI_ASSERT_DOM_LOCKED();
  if (lines_.empty()) {
    DCHECK_EQ(lines_height_, 0.0f);
    line->set_origin(bounds_.origin());
  } else {
    DCHECK(lines_.back()->GetEnd() == line->GetStart());
    if (!dirty_line_point_) {
      auto const last_line = lines_.back();
      line->set_origin(gfx::PointF(last_line->left(), last_line->bottom()));
    }
  }
  lines_.push_back(line);
  lines_height_ += line->GetHeight();
}

bool TextBlock::DiscardFirstLine() {
  UI_ASSERT_DOM_LOCKED();
  if (lines_.empty())
    return false;

  auto const line = lines_.front();
  lines_height_ -= line->GetHeight();
  lines_.pop_front();
  dirty_line_point_ = true;
  return true;
}

bool TextBlock::DiscardLastLine() {
  UI_ASSERT_DOM_LOCKED();
  if (lines_.empty())
    return false;

  auto const line = lines_.back();
  lines_height_ -= line->GetHeight();
  lines_.pop_back();
  return true;
}

text::Posn TextBlock::EndOfLine(text::Posn text_offset) {
  UI_ASSERT_DOM_LOCKED();

  if (text_offset >= text_buffer_->GetEnd())
    return text_buffer_->GetEnd();

  InvalidateCache();
  if (auto const line = text_line_cache_->FindLine(text_offset))
    return line->GetEnd() - 1;

  auto start_offset = text_buffer_->ComputeStartOfLine(text_offset);
  TextFormatter formatter(text_buffer_, start_offset, bounds_, zoom_);
  for (;;) {
    auto const line = FormatLine(&formatter);
    if (text_offset < line->GetEnd())
      return line->GetEnd() - 1;
  }
}

void TextBlock::EnsureLinePoints() {
  UI_ASSERT_DOM_LOCKED();
  if (!dirty_line_point_)
    return;
  auto line_top = bounds_.top;
  for (auto line : lines_) {
    line->set_origin(gfx::PointF(bounds_.left, line_top));
    line_top = line->bottom();
  }

  dirty_line_point_ = false;
}

void TextBlock::Format(text::Posn text_offset) {
  UI_ASSERT_DOM_LOCKED();
  InvalidateCache();
  lines_.clear();
  lines_height_ = 0;
  dirty_ = false;
  dirty_line_point_ = false;

  // TODO(eval1749) We should recompute default style when style is changed,
  // rather than every |Format| call.
  const auto& style = text_buffer_->GetDefaultStyle();
  // |default_style_| is used for providing fill color of right of end of line
  // and bottom of text block.
  default_style_ = RenderStyle(style, nullptr);

  auto const line_start = text_buffer_->ComputeStartOfLine(text_offset);
  TextFormatter formatter(text_buffer_, line_start, bounds_, zoom_);
  for (;;) {
    auto const line = FormatLine(&formatter);
    DCHECK_GT(line->bounds().height(), 0.0f);
    Append(line);

    // Line must have at least one cell other than filler.
    DCHECK_GE(line->GetEnd(), line->GetStart());

    if (lines_height_ >= bounds_.height()) {
      // TextBlock is filled up with lines.
      break;
    }

    if (line->GetEnd() > text_buffer_->GetEnd()) {
      // We have no more contents.
      break;
    }
  }

  // Scroll up until we have |text_offset| in this |TextBlock|.
  while (text_offset > lines_.front()->GetEnd()) {
    DiscardFirstLine();
    auto const line = FormatLine(&formatter);
    Append(line);
    if (line->GetEnd() >= text_buffer_->GetEnd())
      break;
  }
  EnsureLinePoints();
  view_start_ = lines_.front()->GetStart();
  ++format_counter_;
}

bool TextBlock::FormatIfNeeded() {
  if (!ShouldFormat())
    return false;
  Format(view_start_);
  return true;
}

TextLine* TextBlock::FormatLine(TextFormatter* formatter) {
  UI_ASSERT_DOM_LOCKED();
  auto const cached_line = text_line_cache_->FindLine(
      formatter->text_offset());
  if (cached_line) {
    formatter->set_text_offset(cached_line->GetEnd());
    return cached_line;
  }
  auto const line = formatter->FormatLine();
  text_line_cache_->Register(line);
  return line;
}

text::Posn TextBlock::GetEnd() {
  UI_ASSERT_DOM_LOCKED();
  FormatIfNeeded();
  return lines_.back()->GetEnd();
}

text::Posn TextBlock::GetStart() {
  UI_ASSERT_DOM_LOCKED();
  FormatIfNeeded();
  return lines_.front()->GetStart();
}

text::Posn TextBlock::GetVisibleEnd() {
  UI_ASSERT_DOM_LOCKED();
  FormatIfNeeded();
  DCHECK(!dirty_line_point_);
  for (auto it = lines_.crbegin(); it != lines_.crend(); ++it) {
    auto const line = *it;
    if (line->bounds().bottom <= bounds_.bottom)
      return line->GetEnd();
  }
  return lines_.front()->GetEnd();
}

gfx::RectF TextBlock::HitTestTextPosition(text::Posn text_offset) {
  UI_ASSERT_DOM_LOCKED();
  InvalidateCache();
  if (auto const line = text_line_cache_->FindLine(text_offset))
    return line->HitTestTextPosition(text_offset);

  auto start_offset = text_buffer_->ComputeStartOfLine(text_offset);
  TextFormatter formatter(text_buffer_, start_offset, bounds_, zoom_);
  for (;;) {
    auto const line = FormatLine(&formatter);
    if (text_offset < line->GetEnd())
      return line->HitTestTextPosition(text_offset);
  }
}

void TextBlock::InvalidateCache() {
  UI_ASSERT_DOM_LOCKED();
  text_line_cache_->Invalidate(bounds_, zoom_);
  if (!dirty_)
    return;
  lines_.clear();
  lines_height_ = 0;
}

void TextBlock::InvalidateLines(text::Posn offset) {
  ASSERT_DOM_LOCKED();
  text_line_cache_->DidChangeBuffer(offset);
  dirty_ = true;
}

bool TextBlock::IsPositionFullyVisible(text::Posn offset) {
  UI_ASSERT_DOM_LOCKED();
  FormatIfNeeded();
  return offset >= GetStart() && offset < GetVisibleEnd();
}

bool TextBlock::IsShowEndOfDocument() {
  UI_ASSERT_DOM_LOCKED();
  DCHECK(!dirty_);
  DCHECK(!dirty_line_point_);
  auto const last_line = lines_.back();
  return last_line->GetEnd() > text_buffer_->GetEnd() &&
         last_line->bottom() <= bounds_.bottom;
}

text::Posn TextBlock::MapPointToPosition(gfx::PointF point) {
  UI_ASSERT_DOM_LOCKED();
  FormatIfNeeded();

  if (point.y < bounds_.top)
    return GetStart();
  if (point.y >= bounds_.bottom)
    return GetEnd();

  auto line_top = bounds_.top;
  for (const auto line : lines_) {
    auto const y = point.y - line_top;
    line_top += line->GetHeight();

    if (y >= line->GetHeight())
      continue;

    auto cell_left = bounds_.left;
    if (point.x < cell_left)
      return line->GetStart();

    auto result_offset = line->GetEnd() - 1;
    for (const auto cell : line->cells()) {
      auto x = point.x - cell_left;
      cell_left += cell->width();
      const auto offset = cell->MapXToPosn(x);
      if (offset >= 0)
        result_offset = offset;
      if (x >= 0 && x < cell->width())
        break;
    }
    return result_offset;
  }
  return GetEnd() - 1;
}

text::Posn TextBlock::MapPointXToOffset(text::Posn text_offset, float point_x) {
  UI_ASSERT_DOM_LOCKED();
  InvalidateCache();
  if (auto const line = text_line_cache_->FindLine(text_offset))
    return line->MapXToPosn(point_x);

  auto start_offset = text_buffer_->ComputeStartOfLine(text_offset);
  TextFormatter formatter(text_buffer_, start_offset, bounds_, zoom_);
  for (;;) {
    auto const line = FormatLine(&formatter);
    if (text_offset < line->GetEnd())
      return line->MapXToPosn(point_x);
  }
}

void TextBlock::Prepend(TextLine* line) {
  UI_ASSERT_DOM_LOCKED();
  lines_.push_front(line);
  lines_height_ += line->GetHeight();
  dirty_line_point_ = true;
}

bool TextBlock::ScrollDown() {
  FormatIfNeeded();
  if (!lines_.front()->GetStart())
    return false;
  auto const goal_offset = lines_.front()->GetStart() - 1;
  auto const start_offset = text_buffer_->ComputeStartOfLine(goal_offset);
  TextFormatter formatter(text_buffer_, start_offset, bounds_, zoom_);
  for (;;) {
    auto const line = FormatLine(&formatter);
    if (goal_offset < line->GetEnd()) {
      Prepend(line);
      break;
    }
  }

  // Discard lines outside of screen.
  EnsureLinePoints();
  while (lines_.back()->top() >= bounds_.bottom) {
    DiscardLastLine();
  }
  view_start_ = lines_.front()->GetStart();
  return true;
}

bool TextBlock::ScrollToPosition(text::Posn offset) {
  FormatIfNeeded();

  if (IsPositionFullyVisible(offset))
    return false;

  const auto scrollable = bounds_.height() / 2;

  if (offset > GetStart()) {
    auto scrolled = 0.0f;
    while (scrolled < scrollable) {
      const auto scroll_height = lines_.front()->height();
      if (!ScrollUp())
        return scrolled > 0.0f;
      if (IsPositionFullyVisible(offset))
        return true;
      scrolled += scroll_height;
    }
  } else {
    auto scrolled = 0.0f;
    while (scrolled < scrollable) {
      auto const scroll_height = lines_.back()->height();
      if (!ScrollDown())
        return scrolled > 0.0f;
      if (IsPositionFullyVisible(offset))
        return true;
      scrolled += scroll_height;
    }
  }

  Format(offset);
  // When start of line is very far from |offset|, |offset| is below view port.
  // We scroll up until |offset| is in view port.
  while (!IsPositionFullyVisible(offset)) {
    if (!ScrollUp())
      return true;
  }

  // If this page shows end of buffer, we shows lines as much as
  // possible to fit in page.
  auto const buffer_end = text_buffer_->GetEnd();
  if (GetEnd() >= buffer_end) {
    while (IsPositionFullyVisible(buffer_end)) {
      if (!ScrollDown())
        return true;
    }
    ScrollUp();
    return true;
  }

  // Move line containing |offset| to middle of screen.
  auto scrolled = HitTestTextPosition(offset).top - bounds_.top;
  if (scrolled < scrollable) {
    while (scrolled < scrollable) {
      auto const scroll_height = lines_.front()->height();
      if (!ScrollDown())
        return true;
      scrolled += scroll_height;
    }
    return true;
  }

  scrolled = scrollable + scrolled;
  while (scrolled < scrollable) {
    auto const scroll_height = lines_.back()->height();
    if (!ScrollUp())
      return true;
    scrolled += scroll_height;
  }
  return true;
}

bool TextBlock::ScrollUp() {
  FormatIfNeeded();
  EnsureLinePoints();
  if (IsShowEndOfDocument())
    return false;

  if (!DiscardFirstLine())
    return false;

  view_start_ = lines_.front()->GetStart();
  EnsureLinePoints();
  if (IsShowEndOfDocument())
    return true;

  auto const start_offset = lines_.back()->GetEnd();
  TextFormatter formatter(text_buffer_, start_offset, bounds_, zoom_);
  auto const line = FormatLine(&formatter);
  Append(line);
  return true;
}

void TextBlock::SetBounds(const gfx::RectF& new_bounds) {
  DCHECK(!new_bounds.empty());
  if (bounds_ == new_bounds)
    return;
  bounds_ = new_bounds;
  dirty_ = true;
}

void TextBlock::SetZoom(float new_zoom) {
  DCHECK_GT(new_zoom, 0.0f);
  if (zoom_ == new_zoom)
    return;
  zoom_ = new_zoom;
  dirty_ = true;
}

bool TextBlock::ShouldFormat() const {
  UI_ASSERT_DOM_LOCKED();
  return dirty_;
}

text::Posn TextBlock::StartOfLine(text::Posn text_offset) {
  UI_ASSERT_DOM_LOCKED();

  if (text_offset <= 0)
    return 0;

  InvalidateCache();
  if (auto const line = text_line_cache_->FindLine(text_offset))
    return line->GetStart();

  auto start_offset = text_buffer_->ComputeStartOfLine(text_offset);
  if (!start_offset)
    return 0;

  TextFormatter formatter(text_buffer_, start_offset, bounds_, zoom_);
  for (;;) {
    auto const line = FormatLine(&formatter);
    start_offset = line->GetEnd();
    if (text_offset < start_offset)
      return line->GetStart();
  }
}

// text::BufferMutationObserver
void TextBlock::DidChangeStyle(text::Posn offset, size_t) {
  ASSERT_DOM_LOCKED();
  InvalidateLines(offset);
}

void TextBlock::DidDeleteAt(text::Posn offset, size_t length) {
  ASSERT_DOM_LOCKED();
  InvalidateLines(offset);
  if (view_start_ <= offset)
    return;
  view_start_ = std::max(static_cast<text::Posn>(view_start_ - length),
                         offset);
}

void TextBlock::DidInsertAt(text::Posn offset, size_t length) {
  ASSERT_DOM_LOCKED();
  InvalidateLines(offset);
  if (view_start_ <= offset)
    return;
  view_start_ += length;
}

}  // namespace rendering
}  // namespace views
