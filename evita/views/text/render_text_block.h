// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_text_render_text_block_h)
#define INCLUDE_evita_views_text_render_text_block_h

#include <list>

#include "evita/gfx_base.h"
#include "evita/text/buffer_mutation_observer.h"
#include "evita/views/text/render_style.h"
#include "evita/vi_style.h"

namespace views {
namespace rendering {

class RenderStyle;
class TextLine;

class TextBlock : public text::BufferMutationObserver {
  private: RenderStyle default_style_;
  private: bool dirty_;
  private: bool dirty_line_point_;
  private: float m_cy;
  private: std::list<TextLine*> lines_;
  private: gfx::RectF rect_;
  private: text::Buffer* const text_buffer_;

  public: TextBlock(text::Buffer* buffer);
  public: ~TextBlock();

  public: float bottom() const { return rect_.bottom; }
  public: const RenderStyle& default_style() const { return default_style_; }
  public: void set_default_style(const RenderStyle& style) {
    default_style_ = style;
  }
  public: bool dirty() const { return dirty_; }
  public: float height() const { return rect_.height(); }
  public: float left() const { return rect_.left; }
  public: const std::list<TextLine*>& lines() const { return lines_; }
  public: const gfx::RectF& rect() const { return rect_; }
  public: float right() const { return rect_.right; }
  public: const text::Buffer* text_buffer() const { return text_buffer_; }
  public: float top() const { return rect_.top; }
  public: float width() const { return rect_.width(); }

  public: void Append(TextLine*);
  // Returns true if discarded the first line.
  public: bool DiscardFirstLine();
  // Returns true if discarded the last line.
  public: bool DiscardLastLine();
  public: void EnsureLinePoints();
  public: void Finish();
  public: TextLine* GetFirst() const { return lines_.front(); }
  public: float GetHeight() const { return m_cy; }
  public: TextLine* GetLast() const { return lines_.back(); }
  public: text::Posn GetVisibleEnd() const;
  private: void InvalidateLines(text::Posn offset);
  public: bool IsShowEndOfDocument() const;
  public: void Prepend(TextLine*);
  public: void Reset();
  public: void SetRect(const gfx::RectF& rect);

  // text::BufferMutationObserver
  private: virtual void DidChangeStyle(Posn offset, size_t length) override;
  private: virtual void DidDeleteAt(Posn offset, size_t length) override;
  private: virtual void DidInsertAt(Posn offset, size_t length) override;

  DISALLOW_COPY_AND_ASSIGN(TextBlock);
};

} // namespace rendering
} // namespace views

#endif //!defined(INCLUDE_evita_views_text_render_text_block_h)
