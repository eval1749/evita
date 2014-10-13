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

namespace base {
class Time;
}

namespace ui {
class CaretOwner;
}

namespace views {
namespace rendering {

class Font;
class TextLine;
class TextBlock;

class ScreenTextBlock final {
  private: class Caret;
  private: class RenderContext;
  friend class RenderContext;

  private: gfx::RectF bounds_;
  private: const std::unique_ptr<Caret> caret_;
  private: bool dirty_;
  private: bool has_screen_bitmap_;
  private: std::vector<TextLine*> lines_;
  private: TextSelection selection_;

  public: explicit ScreenTextBlock(ui::CaretOwner* caret_owner);
  public: virtual ~ScreenTextBlock();

  public: bool dirty() const { return dirty_; }

  private: gfx::RectF HitTestTextPosition(text::Posn offset) const;
  public: void Render(gfx::Canvas* canvas, const TextBlock* text_block,
                      const TextSelection& selection,
                      base::Time now);
  private: void RenderSelection(gfx::Canvas* canvas,
                                const TextSelection& selection,
                                base::Time now);
  public: void RenderSelectionIfNeeded(gfx::Canvas* canvas,
                                       const TextSelection& selection,
                                       base::Time now);
  public: void Reset();
  public: void SetBounds(const gfx::RectF& new_bounds);
  private: void UpdateCaret(gfx::Canvas* canvas, base::Time now);

  DISALLOW_COPY_AND_ASSIGN(ScreenTextBlock);
};

} // namespace rendering
} // namespace views

#endif //!defined(INCLUDE_evita_views_text_screen_text_block_h)
