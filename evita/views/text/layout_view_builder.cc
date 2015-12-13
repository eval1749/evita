// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/text/layout_view_builder.h"

#include "base/logging.h"
#include "evita/text/buffer.h"
#include "evita/views/text/layout_block_flow.h"
#include "evita/views/text/layout_view.h"
#include "evita/views/text/render_font.h"
#include "evita/views/text/render_font_set.h"
#include "evita/views/text/render_selection.h"
#include "evita/views/text/render_style.h"
#include "evita/views/text/root_inline_box.h"
#include "evita/views/text/text_formatter.h"

namespace views {

using Font = rendering::Font;
using FontSet = rendering::FontSet;
using RootInlineBox = rendering::RootInlineBox;
using TextFormatter = rendering::TextFormatter;

LayoutViewBuilder::LayoutViewBuilder(const text::Buffer* buffer,
                                     ui::Caret* caret)
    : buffer_(buffer), caret_(caret), zoom_(1.0f) {}

LayoutViewBuilder::~LayoutViewBuilder() {}

scoped_refptr<LayoutView> LayoutViewBuilder::Build(
    const LayoutBlockFlow& layout_block_flow,
    const TextSelectionModel& selection_model) {
  // TODO(eval1749): We should recompute default style when style is changed,
  // rather than every |Format| call.
  const auto& bgcolor =
      rendering::ColorToColorF(buffer_->GetDefaultStyle().bgcolor());
  const auto& ruler_bounds = ComputeRulerBounds();
  const auto& selection =
      TextFormatter::FormatSelection(buffer_, selection_model);
  if (last_layout_view_ &&
      last_layout_view_->layout_version() ==
          layout_block_flow.format_counter() &&
      last_layout_view_->ruler_bounds() == ruler_bounds &&
      last_layout_view_->selection() == selection &&
      last_layout_view_->bgcolor() == bgcolor) {
    return last_layout_view_;
  }

  std::vector<RootInlineBox*> lines;
  for (const auto& line : layout_block_flow.lines())
    lines.push_back(line->Copy());
  last_layout_view_ = new LayoutView(caret_, layout_block_flow.format_counter(),
                                     layout_block_flow.bounds(), lines,
                                     selection, bgcolor, ruler_bounds);
  return last_layout_view_;
}

gfx::RectF LayoutViewBuilder::ComputeRulerBounds() const {
  // TODO(eval1749): We should expose show/hide and ruler settings to both
  // script and UI.
  auto style = buffer_->GetDefaultStyle();
  style.set_font_size(style.font_size() * zoom_);
  auto const font = FontSet::GetFont(style, 'x');

  auto const num_columns = 81;
  auto const width_of_M = font->GetCharWidth('M');
  auto const ruler_x = ::floor(bounds_.left + width_of_M * num_columns);
  return gfx::RectF(gfx::PointF(ruler_x, bounds_.top),
                    gfx::SizeF(1.0f, bounds_.height()));
}

void LayoutViewBuilder::SetBounds(const gfx::RectF& new_bounds) {
  bounds_ = new_bounds;
}

void LayoutViewBuilder::SetZoom(float new_zoom) {
  DCHECK_GT(new_zoom, 0.0f);
  zoom_ = new_zoom;
}

}  // namespace views
