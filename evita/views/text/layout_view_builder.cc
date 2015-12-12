// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/text/layout_view_builder.h"

#include "evita/text/buffer.h"
#include "evita/views/text/layout_block_flow.h"
#include "evita/views/text/layout_view.h"
#include "evita/views/text/render_selection.h"
#include "evita/views/text/render_style.h"
#include "evita/views/text/root_inline_box.h"
#include "evita/views/text/text_formatter.h"

namespace views {

using RootInlineBox = rendering::RootInlineBox;
using TextFormatter = rendering::TextFormatter;

LayoutViewBuilder::LayoutViewBuilder(const text::Buffer* buffer)
    : buffer_(buffer) {}

LayoutViewBuilder::~LayoutViewBuilder() {}

scoped_refptr<LayoutView> LayoutViewBuilder::Build(
    const LayoutBlockFlow& layout_block_flow,
    const TextSelectionModel& selection_model) {
  // TODO(eval1749): We should recompute default style when style is changed,
  // rather than every |Format| call.
  const auto& bgcolor =
      rendering::ColorToColorF(buffer_->GetDefaultStyle().bgcolor());
  const auto& selection =
      TextFormatter::FormatSelection(buffer_, selection_model);
  if (last_layout_view_ &&
      last_layout_view_->layout_version() ==
          layout_block_flow.format_counter() &&
      last_layout_view_->selection() == selection &&
      last_layout_view_->bgcolor() == bgcolor) {
    return last_layout_view_;
  }

  std::vector<RootInlineBox*> lines;
  for (const auto& line : layout_block_flow.lines())
    lines.push_back(line->Copy());
  last_layout_view_ = new LayoutView(layout_block_flow.format_counter(), lines,
                                     selection, bgcolor);
  return last_layout_view_;
}

}  // namespace views
