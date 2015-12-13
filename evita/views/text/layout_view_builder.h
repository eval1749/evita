// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_TEXT_LAYOUT_VIEW_BUILDER_H_
#define EVITA_VIEWS_TEXT_LAYOUT_VIEW_BUILDER_H_

#include <memory>

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "evita/gfx/rect_f.h"

namespace text {
class Buffer;
}

namespace ui {
class Caret;
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

  explicit LayoutViewBuilder(const text::Buffer* buffer_, ui::Caret* caret);
  ~LayoutViewBuilder();

  scoped_refptr<LayoutView> Build(const LayoutBlockFlow& layout_block_flow,
                                  const TextSelectionModel& selection_model);

  void SetBounds(const gfx::RectF& new_bounds);
  void SetZoom(float new_zoom);

 private:
  gfx::RectF ComputeRulerBounds() const;

  gfx::RectF bounds_;
  const text::Buffer* const buffer_;
  ui::Caret* const caret_;
  scoped_refptr<LayoutView> last_layout_view_;
  float zoom_;

  DISALLOW_COPY_AND_ASSIGN(LayoutViewBuilder);
};

}  // namespace views

#endif  // EVITA_VIEWS_TEXT_LAYOUT_VIEW_BUILDER_H_
