// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/view/view.h"

#include "base/strings/stringprintf.h"
#include "evita/visuals/display/public/display_item_list.h"
#include "evita/visuals/dom/document.h"
#include "evita/visuals/layout/box_finder.h"
#include "evita/visuals/layout/box_tree.h"
#include "evita/visuals/layout/layouter.h"
#include "evita/visuals/layout/root_box.h"
#include "evita/visuals/style/style_tree.h"
#include "evita/visuals/paint/painter.h"
#include "evita/visuals/paint/paint_info.h"
#include "evita/visuals/view/public/selection.h"
#include "evita/visuals/view/public/view_lifecycle.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// View
//
View::View(const Document& document,
           const css::Media& media,
           const std::vector<css::StyleSheet*>& style_sheets)
    : lifecycle_(new ViewLifecycle(document, media)),
      selection_(new Selection(lifecycle_.get())),
      style_tree_(new StyleTree(lifecycle_.get(), style_sheets)),
      box_tree_(new BoxTree(lifecycle_.get(), *selection_, *style_tree_)) {}

View::~View() {
  lifecycle_->StartShutdown();
  box_tree_.reset();
  lifecycle_->FinishShutdown();
}

const Document& View::document() const {
  return lifecycle_->document();
}

const css::Media& View::media() const {
  return lifecycle_->media();
}

HitTestResult View::HitTest(const FloatPoint& point) {
  UpdateLayoutIfNeeded();
  const auto root_box = box_tree_->root_box();
  return BoxFinder(*root_box).FindByPoint(point);
}

std::unique_ptr<DisplayItemList> View::Paint() {
  UpdateStyleIfNeeded();
  UpdateLayoutIfNeeded();

  const auto root_box = box_tree_->root_box();
  if (root_box->IsPaintClean()) {
    // Box tree is changed outside viewport(?).
    return std::unique_ptr<DisplayItemList>();
  }
  const auto& debug_text =
      base::StringPrintf(L"dom: %d, css: %d, box: %d", document().version(),
                         style_tree_->version(), box_tree_->version());
  PaintInfo paint_info(root_box->bounds(), debug_text);
  return std::move(Painter().Paint(paint_info, *root_box));
}

void View::ScheduleForcePaint() {
  box_tree_->ScheduleForcePaint();
}

void View::UpdateLayoutIfNeeded() {
  UpdateStyleIfNeeded();
  box_tree_->UpdateIfNeeded();
  Layouter().Layout(box_tree_->root_box());
}

void View::UpdateStyleIfNeeded() {
  style_tree_->UpdateIfNeeded();
}

}  // namespace visuals
