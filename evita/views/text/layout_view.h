// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_TEXT_LAYOUT_VIEW_H_
#define EVITA_VIEWS_TEXT_LAYOUT_VIEW_H_

#include <vector>

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "evita/gfx_base.h"
#include "evita/views/text/render_selection.h"

namespace views {

namespace rendering {
class RootInlineBox;
};

//////////////////////////////////////////////////////////////////////
//
// LayoutView
//
class LayoutView final : public base::RefCounted<LayoutView> {
 public:
  using RootInlineBox = rendering::RootInlineBox;
  using TextSelection = rendering::TextSelection;

  LayoutView(int layout_version,
             const std::vector<RootInlineBox*>& lines,
             const TextSelection& selection,
             const gfx::ColorF& bgcolor);
  ~LayoutView();

  const gfx::ColorF& bgcolor() const { return bgcolor_; }
  const std::vector<RootInlineBox*>& lines() const { return lines_; }
  const TextSelection& selection() const { return selection_; }
  int layout_version() const { return layout_version_; }

 private:
  const gfx::ColorF bgcolor_;
  const int layout_version_;
  const std::vector<RootInlineBox*> lines_;
  const TextSelection selection_;

  DISALLOW_COPY_AND_ASSIGN(LayoutView);
};

}  // namespace views

#endif  // EVITA_VIEWS_TEXT_LAYOUT_VIEW_H_