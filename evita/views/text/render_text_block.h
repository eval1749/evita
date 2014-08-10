// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_text_render_text_block_h)
#define INCLUDE_evita_views_text_render_text_block_h

#include <list>
#include <memory>

#include "evita/gfx_base.h"
#include "evita/text/buffer_mutation_observer.h"
#include "evita/views/text/render_style.h"

namespace views {
namespace rendering {

class RenderStyle;
class TextFormatter;
class TextLine;

class TextBlock final : public text::BufferMutationObserver {
  private: class TextLineCache;

  private: gfx::RectF bounds_;
  private: RenderStyle default_style_;
  private: bool dirty_;
  private: bool dirty_line_point_;
  private: std::list<TextLine*> lines_;
  private: float lines_height_;
  private: text::Buffer* const text_buffer_;
  private: std::unique_ptr<TextLineCache> text_line_cache_;
  private: text::Posn view_start_;
  private: float zoom_;

  public: explicit TextBlock(text::Buffer* buffer);
  public: virtual ~TextBlock();

  public: const RenderStyle& default_style() const { return default_style_; }
  public: bool dirty() const { return dirty_; }
  public: const std::list<TextLine*>& lines() const { return lines_; }

  private: void Append(TextLine* line);
  // Returns true if discarded the first line.
  private: bool DiscardFirstLine();
  // Returns true if discarded the last line.
  private: bool DiscardLastLine();
  // Returns end of line offset containing |text_offset|.
  public: text::Posn EndOfLine(text::Posn text_offset);
  private: void EnsureLinePoints();
  public: void Format(text::Posn text_offset);
  // Returns true if text format is taken place.
  public: bool FormatIfNeeded();
  private: TextLine* FormatLine(TextFormatter* formatter);
  public: text::Posn GetEnd();
  public: text::Posn GetStart();
  public: text::Posn GetVisibleEnd();
  public: gfx::RectF HitTestTextPosition(text::Posn text_offset);
  private: void InvalidateCache();
  private: void InvalidateLines(text::Posn offset);
  public: bool IsPositionFullyVisible(text::Posn text_offset);
  public: bool IsShowEndOfDocument();
  public: text::Posn MapPointToPosition(gfx::PointF point);
  public: text::Posn MapPointXToOffset(text::Posn text_offset, float point_x);
  private: void Prepend(TextLine* line);
  // Returns true if this |TextBlock| is modified.
  public: bool ScrollDown();
  // Returns true if this |TextBlock| is modified.
  public: bool ScrollToPosition(text::Posn offset);
  // Returns true if this |TextBlock| is modified.
  public: bool ScrollUp();
  public: void SetBounds(const gfx::RectF& new_bounds);
  public: void SetZoom(float new_zoom);
  public: bool ShouldFormat() const;
  // Returns start of line offset containing |text_offset|.
  public: text::Posn StartOfLine(text::Posn text_offset);

  // text::BufferMutationObserver
  private: virtual void DidChangeStyle(Posn offset, size_t length) override;
  private: virtual void DidDeleteAt(Posn offset, size_t length) override;
  private: virtual void DidInsertAt(Posn offset, size_t length) override;

  DISALLOW_COPY_AND_ASSIGN(TextBlock);
};

} // namespace rendering
} // namespace views

#endif //!defined(INCLUDE_evita_views_text_render_text_block_h)
