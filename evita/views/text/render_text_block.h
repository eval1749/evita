// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_text_render_text_block_h)
#define INCLUDE_evita_views_text_render_text_block_h

#include <list>

#include "evita/gfx_base.h"
#include "evita/vi_style.h"

namespace views {
namespace rendering {

class TextLine;

class TextBlock {
  private: bool dirty_;
  private: bool dirty_line_point_;
  private: float m_cy;
  private: std::list<TextLine*> lines_;
  private: gfx::RectF rect_;

  public: TextBlock(const TextBlock& other);
  public: TextBlock();
  public: ~TextBlock();

  public: float bottom() const { return rect_.bottom; }
  public: bool dirty() const { return dirty_; }
  public: float height() const { return rect_.height(); }
  public: float left() const { return rect_.left; }
  public: const std::list<TextLine*>& lines() const { return lines_; }
  public: const gfx::RectF& rect() const { return rect_; }
  public: float right() const { return rect_.right; }
  public: float top() const { return rect_.top; }
  public: float width() const { return rect_.width(); }

  public: void Append(TextLine*);
  public: void EnsureLinePoints();
  public: void Finish();
  public: TextLine* GetFirst() const { return lines_.front(); }
  public: float GetHeight() const { return m_cy; }
  public: TextLine* GetLast() const { return lines_.back(); }
  public: void Prepend(TextLine*);
  public: void Reset();
  public: TextLine* ScrollDown();
  public: TextLine* ScrollUp();
  public: void SetBufferDirtyOffset(Posn offset);
  public: void SetRect(const gfx::RectF& rect);
};

} // namespace rendering
} // namespace views

#endif //!defined(INCLUDE_evita_views_text_render_text_block_h)

