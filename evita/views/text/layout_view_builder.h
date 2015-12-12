// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_TEXT_LAYOUT_VIEW_BUILDER_H_
#define EVITA_VIEWS_TEXT_LAYOUT_VIEW_BUILDER_H_

#include <memory>

#include "base/macros.h"
#include "base/memory/ref_counted.h"

namespace text {
class Buffer;
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

  explicit LayoutViewBuilder(const text::Buffer* buffer_);
  ~LayoutViewBuilder();

  scoped_refptr<LayoutView> Build(const LayoutBlockFlow& layout_block_flow,
                                  const TextSelectionModel& selection_model);

 private:
  const text::Buffer* const buffer_;
  scoped_refptr<LayoutView> last_layout_view_;

  DISALLOW_COPY_AND_ASSIGN(LayoutViewBuilder);
};

}  // namespace views

#endif  // EVITA_VIEWS_TEXT_LAYOUT_VIEW_BUILDER_H_
