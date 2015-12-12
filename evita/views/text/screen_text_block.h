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
class Caret;
}

namespace views {
class LayoutView;

namespace rendering {
class Font;
class RootInlineBox;

class ScreenTextBlock final {
 public:
  explicit ScreenTextBlock(ui::Caret* caret);
  ~ScreenTextBlock();

  void Paint(gfx::Canvas* canvas,
             base::Time now,
             const LayoutView* last_layout_view,
             const LayoutView& layout_view);
  void Reset();

 private:
  void PaintSelection(gfx::Canvas* canvas,
                      base::Time now,
                      const LayoutView& layout_view);
  void PaintSelectionIfNeeded(gfx::Canvas* canvas,
                              base::Time now,
                              const LayoutView* last_layout_view,
                              const LayoutView& layout_view);
  void UpdateCaret(gfx::Canvas* canvas,
                   base::Time now,
                   const LayoutView& layout_view);

  ui::Caret* const caret_;

  DISALLOW_COPY_AND_ASSIGN(ScreenTextBlock);
};

}  // namespace rendering
}  // namespace views

#endif  // EVITA_VIEWS_TEXT_SCREEN_TEXT_BLOCK_H_
