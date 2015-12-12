// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_TEXT_SCREEN_TEXT_BLOCK_H_
#define EVITA_VIEWS_TEXT_SCREEN_TEXT_BLOCK_H_

#include <memory>
#include <vector>

#include "base/basictypes.h"
#include "base/memory/ref_counted.h"
#include "evita/ed_defs.h"
#include "evita/gfx_base.h"

namespace base {
class Time;
}

namespace ui {
class CaretOwner;
}

namespace views {
class LayoutView;

namespace rendering {
class Font;
class RootInlineBox;

class ScreenTextBlock final {
 public:
  explicit ScreenTextBlock(ui::CaretOwner* caret_owner);
  ~ScreenTextBlock();

  void Paint(gfx::Canvas* canvas,
             scoped_refptr<LayoutView> layout_view,
             base::Time now);
  void PaintSelectionIfNeeded(gfx::Canvas* canvas,
                              scoped_refptr<LayoutView> layout_view,
                              base::Time now);
  void Reset();
  void SetBounds(const gfx::RectF& new_bounds);

 private:
  class Caret;

  gfx::RectF HitTestTextPosition(text::Posn offset) const;
  void PaintSelection(gfx::Canvas* canvas, base::Time now);
  void UpdateCaret(gfx::Canvas* canvas, base::Time now);

  gfx::RectF bounds_;
  const std::unique_ptr<Caret> caret_;
  // The |LayoutView| in screen.
  scoped_refptr<LayoutView> layout_view_;

  DISALLOW_COPY_AND_ASSIGN(ScreenTextBlock);
};

}  // namespace rendering
}  // namespace views

#endif  // EVITA_VIEWS_TEXT_SCREEN_TEXT_BLOCK_H_
