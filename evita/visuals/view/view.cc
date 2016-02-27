// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/view/view.h"

#include "base/strings/stringprintf.h"
#include "base/trace_event/trace_event.h"
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
#include "evita/visuals/view/view_observer.h"

namespace visuals {

namespace {

bool ShouldNotifyViewChange(ViewLifecycle::State new_state,
                            ViewLifecycle::State old_state) {
  if (old_state == ViewLifecycle::State::PaintClean)
    return static_cast<int>(new_state) < static_cast<int>(old_state);
  return new_state == ViewLifecycle::State::VisualUpdatePending;
}

}  // namespace

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
      box_tree_(new BoxTree(lifecycle_.get(), *selection_, *style_tree_)) {
  lifecycle_->AddObserver(this);
}

View::~View() {
  lifecycle_->RemoveObserver(this);
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

void View::AddObserver(ViewObserver* observer) const {
  observers_.AddObserver(observer);
}

FloatQuad View::ComputeBorderBoxQuad(const Node& node) {
  TRACE_EVENT0("visuals", "View::ComputeBorderBoxQuad");
  UpdateLayoutIfNeeded();
  const auto box = box_tree_->BoxFor(node);
  return box ? FloatQuad(box->bounds()) : FloatQuad();
}

HitTestResult View::HitTest(const FloatPoint& point) {
  TRACE_EVENT0("visuals", "View::HitTest");
  UpdateLayoutIfNeeded();
  const auto root_box = box_tree_->root_box();
  return BoxFinder(*root_box).FindByPoint(point);
}

std::unique_ptr<DisplayItemList> View::Paint() {
  TRACE_EVENT0("visuals", "View::Paint");
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
  auto display_item_list = Painter().Paint(paint_info, *root_box);
  // TODO(eval1749): Do we have another place to call |Selection::DidPaint()|.
  // In here, we call |Selection::DidPaint()| even if painter doesn't paint
  // caret, e.g. selection is none, selection is out side of viewport.
  selection_->DidPaint();
  return std::move(display_item_list);
}

void View::RemoveObserver(ViewObserver* observer) const {
  observers_.RemoveObserver(observer);
}

void View::ScheduleForcePaint() {
  box_tree_->ScheduleForcePaint();
}

void View::Start() {
  ViewLifecycle::Scope(lifecycle_.get(), ViewLifecycle::State::Started);
}

void View::UpdateLayoutIfNeeded() {
  TRACE_EVENT0("visuals", "View::UpdateLayoutIfNeeded");
  UpdateStyleIfNeeded();
  box_tree_->UpdateIfNeeded();
  Layouter().Layout(box_tree_->root_box());
}

void View::UpdateStyleIfNeeded() {
  TRACE_EVENT0("visuals", "View::UpdateStyleIfNeeded");
  style_tree_->UpdateIfNeeded();
}

// ViewLifecycleObserver
void View::DidChangeLifecycleState(ViewLifecycle::State new_state,
                                   ViewLifecycle::State old_state) {
  if (!ShouldNotifyViewChange(new_state, old_state))
    return;
  FOR_EACH_OBSERVER(ViewObserver, observers_, DidChangeView());
}

}  // namespace visuals
