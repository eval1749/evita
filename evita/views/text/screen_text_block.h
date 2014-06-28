// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_text_screen_text_block_h)
#define INCLUDE_evita_views_text_screen_text_block_h

#include <memory>
#include <vector>

#include "base/basictypes.h"
#include "evita/gfx_base.h"
#include "evita/views/text/render_selection.h"
#include "evita/vi_style.h"

namespace views {
namespace rendering {

class TextLine;
class TextBlock;

class ScreenTextBlock final : public gfx::Canvas::Observer {
  private: class Caret;
  private: class RenderContext;
  friend class RenderContext;

  private: gfx::RectF bounds_;
  private: std::unique_ptr<Caret> caret_;
  private: bool dirty_;
  private: gfx::Canvas* gfx_;
  private: bool has_screen_bitmap_;
  private: std::vector<TextLine*> lines_;
  private: TextSelection selection_;

  public: ScreenTextBlock();
  public: virtual ~ScreenTextBlock();

  public: bool dirty() const { return dirty_; }

  public: void DidKillFocus();
  public: void DidSetFocus();
  private: gfx::RectF HitTestTextPosition(text::Posn offset) const;
  public: void Render(const TextBlock* text_block,
                      const TextSelection& selection);
  private: void RenderCaret();
  private: void RenderSelection(const TextSelection& selection);
  public: void RenderSelectionIfNeeded(const TextSelection& selection);
  public: void Reset();
  public: void SetGraphics(const gfx::Canvas* gfx);
  public: void SetBounds(const gfx::RectF& rect);

  // gfx::Canvas::Observer
  private: virtual void ShouldDiscardResources() override;

  DISALLOW_COPY_AND_ASSIGN(ScreenTextBlock);
};

} // namespace rendering
} // namespace views

#endif //!defined(INCLUDE_evita_views_text_screen_text_block_h)
