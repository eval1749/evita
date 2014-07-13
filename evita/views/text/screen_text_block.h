// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_text_screen_text_block_h)
#define INCLUDE_evita_views_text_screen_text_block_h

#include <memory>
#include <vector>

#include "base/basictypes.h"
#include "evita/gfx_base.h"
#include "evita/ui/caret.h"
#include "evita/views/text/render_selection.h"

namespace views {
namespace rendering {

class Font;
class TextLine;
class TextBlock;

class ScreenTextBlock final : private gfx::Canvas::Observer,
                              private ui::Caret::Delegate {
  private: class RenderContext;
  friend class RenderContext;

  private: gfx::RectF bounds_;
  private: gfx::Canvas* canvas_;
  private: bool dirty_;
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
  public: void SetBounds(const gfx::RectF& rect);
  public: void SetCanvas(gfx::Canvas* canvas);

  // gfx::Canvas::Observer
  private: virtual void ShouldDiscardResources() override;


  // ui::Caret::Delegate
  private: virtual void HideCaret(gfx::Canvas* canvas,
                                  const ui::Caret& caret) override;
  private: virtual void PaintCaret(gfx::Canvas* canvas,
                                   const ui::Caret& caret) override;
  private: virtual void ShowCaret(gfx::Canvas* canvas,
                                  const ui::Caret& caret) override;

  DISALLOW_COPY_AND_ASSIGN(ScreenTextBlock);
};

} // namespace rendering
} // namespace views

#endif //!defined(INCLUDE_evita_views_text_screen_text_block_h)
