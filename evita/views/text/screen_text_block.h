// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_TEXT_SCREEN_TEXT_BLOCK_H_
#define EVITA_VIEWS_TEXT_SCREEN_TEXT_BLOCK_H_

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
class PaintTextBlock;

namespace rendering {
class Font;
class RootInlineBox;

class ScreenTextBlock final {
 public:
  explicit ScreenTextBlock(ui::CaretOwner* caret_owner);
  ~ScreenTextBlock();

  bool dirty() const { return dirty_; }

  void Paint(gfx::Canvas* canvas,
             const PaintTextBlock& paint_text_block,
             base::Time now);
  void PaintSelectionIfNeeded(gfx::Canvas* canvas,
                              const TextSelection& new_selection,
                              base::Time now);
  void Reset();
  void SetBounds(const gfx::RectF& new_bounds);

 private:
  class Caret;
  class PaintContext;

  gfx::RectF HitTestTextPosition(text::Posn offset) const;
  void PaintSelection(gfx::Canvas* canvas,
                      const TextSelection& selection,
                      base::Time now);
  void UpdateCaret(gfx::Canvas* canvas, base::Time now);

  gfx::RectF bounds_;
  const std::unique_ptr<Caret> caret_;
  bool dirty_;
  bool has_screen_bitmap_;
  std::vector<RootInlineBox*> lines_;
  TextSelection selection_;

  DISALLOW_COPY_AND_ASSIGN(ScreenTextBlock);
};

}  // namespace rendering
}  // namespace views

#endif  // EVITA_VIEWS_TEXT_SCREEN_TEXT_BLOCK_H_
