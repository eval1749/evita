// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_LAYOUT_PAINT_VIEW_BUILDER_H_
#define EVITA_LAYOUT_PAINT_VIEW_BUILDER_H_

#include <memory>

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/time/time.h"
#include "base/timer/timer.h"
#include "evita/gfx/rect_f.h"

namespace paint {
enum class CaretState;
class Caret;
class View;
}

namespace text {
class Buffer;
}

namespace ui {
class AnimatableWindow;
}

namespace layout {

class BlockFlow;
class RenderStyle;
class TextSelectionModel;

//////////////////////////////////////////////////////////////////////
//
// PaintViewBuilder
//
class PaintViewBuilder final {
 public:
  explicit PaintViewBuilder(const BlockFlow& block,
                            ui::AnimatableWindow* caret_owner);
  ~PaintViewBuilder();

  scoped_refptr<paint::View> Build(const TextSelectionModel& selection_model,
                                   const base::TimeTicks& now);

  void SetBounds(const gfx::RectF& new_bounds);
  void SetZoom(float new_zoom);

 private:
  gfx::RectF PaintViewBuilder::ComputeCaretBounds(
      const TextSelectionModel& selection_model) const;
  paint::CaretState ComputeCaretState(const gfx::RectF& bounds,
                                      const base::TimeTicks& now) const;

  gfx::RectF ComputeRulerBounds() const;
  void DidFireCaretTimer();
  void StartCaretTimer();
  void StopCaretTimer();

  gfx::RectF bounds_;
  const BlockFlow& block_;
  gfx::RectF caret_bounds_;
  ui::AnimatableWindow* const caret_owner_;
  paint::CaretState caret_state_;
  base::TimeTicks caret_time_;
  base::RepeatingTimer caret_timer_;
  float zoom_;

  DISALLOW_COPY_AND_ASSIGN(PaintViewBuilder);
};

}  // namespace layout

#endif  // EVITA_LAYOUT_PAINT_VIEW_BUILDER_H_
