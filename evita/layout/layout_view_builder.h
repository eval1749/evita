// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_LAYOUT_LAYOUT_VIEW_BUILDER_H_
#define EVITA_LAYOUT_LAYOUT_VIEW_BUILDER_H_

#include <memory>

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/time/time.h"
#include "base/timer/timer.h"
#include "evita/gfx/rect_f.h"
#include "evita/layout/layout_caret.h"

namespace text {
class Buffer;
}

namespace ui {
class AnimatableWindow;
}

namespace views {

namespace rendering {
class RenderStyle;
class LayoutBlockFlow;
class TextSelectionModel;
}

class LayoutView;

//////////////////////////////////////////////////////////////////////
//
// LayoutViewBuilder
//
class LayoutViewBuilder final {
 public:
  using LayoutBlockFlow = rendering::LayoutBlockFlow;
  using RenderStyle = rendering::RenderStyle;
  using TextSelectionModel = rendering::TextSelectionModel;

  explicit LayoutViewBuilder(const text::Buffer* buffer_,
                             ui::AnimatableWindow* caret_owner);
  ~LayoutViewBuilder();

  scoped_refptr<LayoutView> Build(const LayoutBlockFlow& layout_block_flow,
                                  const TextSelectionModel& selection_model,
                                  base::Time now);

  void SetBounds(const gfx::RectF& new_bounds);
  void SetZoom(float new_zoom);

 private:
  gfx::RectF LayoutViewBuilder::ComputeCaretBounds(
      const LayoutBlockFlow& layout_block_flow,
      const TextSelectionModel& selection_model) const;
  LayoutCaret::State ComputeCaretState(const gfx::RectF& bounds,
                                       base::Time now) const;

  gfx::RectF ComputeRulerBounds() const;
  void DidFireCaretTimer();
  void StartCaretTimer();
  void StopCaretTimer();

  gfx::RectF bounds_;
  const text::Buffer* const buffer_;
  gfx::RectF caret_bounds_;
  ui::AnimatableWindow* const caret_owner_;
  LayoutCaret::State caret_state_;
  base::Time caret_time_;
  base::RepeatingTimer caret_timer_;
  float zoom_;

  DISALLOW_COPY_AND_ASSIGN(LayoutViewBuilder);
};

}  // namespace views

#endif  // EVITA_LAYOUT_LAYOUT_VIEW_BUILDER_H_
