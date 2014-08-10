// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_text_render_text_block_h)
#define INCLUDE_evita_views_text_render_text_block_h

#include <list>

#include "evita/gfx_base.h"
#include "evita/text/buffer_mutation_observer.h"
#include "evita/views/text/render_style.h"

namespace views {
namespace rendering {

class RenderStyle;
class TextLine;

class TextBlock final : public text::BufferMutationObserver {
  private: gfx::RectF bounds_;
  private: RenderStyle default_style_;
  private: bool dirty_;
  private: bool dirty_line_point_;
  private: std::list<TextLine*> lines_;
  private: float lines_height_;
  private: text::Buffer* const text_buffer_;
  private: float zoom_;

  public: TextBlock(text::Buffer* buffer, const gfx::RectF& bounds, float zoom);
  public: explicit TextBlock(text::Buffer* buffer);
  public: virtual ~TextBlock();

  public: float bottom() const { return bounds_.bottom; }
  public: const RenderStyle& default_style() const { return default_style_; }
  public: bool dirty() const { return dirty_; }
  public: float height() const { return bounds_.height(); }
  public: float left() const { return bounds_.left; }
  public: const std::list<TextLine*>& lines() const { return lines_; }
  public: const gfx::RectF& bounds() const { return bounds_; }
  public: float right() const { return bounds_.right; }
  public: const text::Buffer* text_buffer() const { return text_buffer_; }
  public: float top() const { return bounds_.top; }
  public: float width() const { return bounds_.width(); }
  public: float zoom() const { return zoom_; }

  public: void Append(TextLine*);
  // Returns true if discarded the first line.
  public: bool DiscardFirstLine();
  // Returns true if discarded the last line.
  public: bool DiscardLastLine();
  // Returns end of line offset containing |text_offset|.
  public: text::Posn EndOfLine(text::Posn text_offset) const;
  public: void EnsureLinePoints();
  private: TextLine* FindLine(text::Posn text_offset) const;
  public: void Format(text::Posn text_offset, const gfx::RectF& bounds,
                      float zoom);
  public: TextLine* GetFirst() const { return lines_.front(); }
  public: float GetHeight() const { return lines_height_; }
  public: TextLine* GetLast() const { return lines_.back(); }
  public: text::Posn GetVisibleEnd() const;
  public: gfx::RectF HitTestTextPosition(text::Posn text_offset) const;
  private: void InvalidateLines(text::Posn offset);
  public: bool IsShowEndOfDocument() const;
  public: text::Posn MapPointXToOffset(text::Posn text_offset,
                                       float point_x) const;
  public: void Prepend(TextLine*);
  public: void Reset();
  // Returns true if this |TextBlock| is modified.
  public: bool ScrollDown();
  // Returns true if this |TextBlock| is modified.
  public: bool ScrollUp();
  public: bool ShouldFormat(const gfx::RectF& bounds, float zoom) const;
  // Returns start of line offset containing |text_offset|.
  public: text::Posn StartOfLine(text::Posn text_offset) const;

  // text::BufferMutationObserver
  private: virtual void DidChangeStyle(Posn offset, size_t length) override;
  private: virtual void DidDeleteAt(Posn offset, size_t length) override;
  private: virtual void DidInsertAt(Posn offset, size_t length) override;

  DISALLOW_COPY_AND_ASSIGN(TextBlock);
};

} // namespace rendering
} // namespace views

#endif //!defined(INCLUDE_evita_views_text_render_text_block_h)
