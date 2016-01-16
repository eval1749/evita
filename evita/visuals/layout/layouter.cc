// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <vector>

#include "evita/visuals/layout/layouter.h"

#include "base/logging.h"
#include "evita/visuals/geometry/float_rect.h"
#include "evita/visuals/layout/border.h"
#include "evita/visuals/layout/box_editor.h"
#include "evita/visuals/layout/box_visitor.h"
#include "evita/visuals/layout/flow_box.h"
#include "evita/visuals/layout/margin.h"
#include "evita/visuals/layout/padding.h"
#include "evita/visuals/layout/root_box.h"
#include "evita/visuals/layout/size_calculator.h"

namespace visuals {

namespace {

// TODO(eval1749) We should move |IsDisplayOutsideInline()| to |css::Display|.
bool IsDisplayOutsideInline(const css::Display& display) {
  return display.is_inline() || display.is_inline_block();
}

//////////////////////////////////////////////////////////////////////
//
// LayoutVisitor
//
class LayoutVisitor final : public BoxVisitor {
 public:
  LayoutVisitor() = default;
  ~LayoutVisitor() final = default;

  void LayoutIfNeeded(Box* box);

 private:
  void Layout(Box* box, const FloatPoint& origin, const FloatSize& size);
  void Layout(Box* box, const FloatRect& bounds);
  void Layout(Box* box);
  void LayoutFlowBoxHorizontally(const FlowBox& flow_box);
  void LayoutFlowBoxVertically(const FlowBox& flow_box);

// BoxVisitor
#define V(name) void Visit##name(name* box) final;
  FOR_EACH_VISUAL_BOX(V)
#undef V

  DISALLOW_COPY_AND_ASSIGN(LayoutVisitor);
};

void LayoutVisitor::Layout(Box* box,
                           const FloatPoint& origin,
                           const FloatSize& size) {
  Layout(box, FloatRect(origin, size));
}

void LayoutVisitor::Layout(Box* box, const FloatRect& bounds) {
  BoxEditor().SetBounds(box, bounds);
  Layout(box);
}

void LayoutVisitor::Layout(Box* box) {
  Visit(box);
  BoxEditor().DidLayout(box);
}

void LayoutVisitor::LayoutFlowBoxHorizontally(const FlowBox& flow_box) {
  DCHECK(flow_box.first_child()) << flow_box;
  auto child_origin = FloatPoint();
  for (const auto& child : flow_box.child_boxes()) {
    const auto& child_border = child->ComputeBorder();
    const auto& child_margin = child->ComputeMargin();
    const auto& child_padding = child->ComputePadding();
    const auto& child_size = SizeCalculator().ComputePreferredSize(*child) +
                             child_border.size() + child_padding.size();
    LayoutVisitor().Layout(
        child, FloatRect(child_origin + child_margin.top_left(), child_size));
    child_origin = FloatPoint(child->bounds().right() + child_margin.right(),
                              child_origin.y());
  }
}

void LayoutVisitor::LayoutFlowBoxVertically(const FlowBox& flow_box) {
  DCHECK(flow_box.first_child()) << flow_box;
  auto child_origin = FloatPoint();
  const auto content_width = flow_box.content_bounds().width();
  for (const auto& child : flow_box.child_boxes()) {
    const auto& child_border = child->ComputeBorder();
    const auto& child_margin = child->ComputeMargin();
    const auto& child_padding = child->ComputePadding();
    const auto& child_size = SizeCalculator().ComputePreferredSize(*child) +
                             child_border.size() + child_padding.size();
    if (child->position().is_absolute()) {
      LayoutVisitor().Layout(child, FloatPoint(child->left().length().value(),
                                               child->top().length().value()) +
                                        child_margin.top_left(),
                             FloatSize(content_width, child_size.height()));
      continue;
    }
    LayoutVisitor().Layout(
        child, FloatRect(child_origin + child_margin.top_left(),
                         FloatSize(content_width, child_size.height())));
    child_origin = FloatPoint(child_origin.x(),
                              child->bounds().bottom() + child_margin.bottom());
  }
}

void LayoutVisitor::LayoutIfNeeded(Box* box) {
  if (box->is_changed())
    return Layout(box);
  const auto container = box->as<ContainerBox>();
  if (!container)
    return;
  if (!container->is_child_changed())
    return;
  for (const auto& child : container->child_boxes())
    LayoutIfNeeded(child);
  return Layout(container);
}

// BoxVisitor
void LayoutVisitor::VisitFlowBox(FlowBox* flow_box) {
  const auto first_child = flow_box->first_child();
  if (!first_child)
    return;
  if (IsDisplayOutsideInline(first_child->display()))
    return LayoutFlowBoxHorizontally(*flow_box);
  return LayoutFlowBoxVertically(*flow_box);
}

void LayoutVisitor::VisitRootBox(RootBox* root) {
  LayoutVisitor().Layout(root->first_child(), root->content_bounds());
}

void LayoutVisitor::VisitTextBox(TextBox* box) {
  BoxEditor().AllocateTextLayout(box);
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Layouter
//
Layouter::Layouter() {}

Layouter::~Layouter() {}

void Layouter::Layout(RootBox* root_box) {
  // TODO(eval1749): Once, we get rid of BoxTreeLifeCycle, we don't need to
  // check |is_changed()| and |is_child_changed()| here.
  if (!root_box->is_changed() && !root_box->is_child_changed())
    return;
  if (root_box->IsLayoutClean())
    return;
  BoxTreeLifecycle::Scope scope(root_box->lifecycle(),
                                BoxTreeLifecycle::State::InLayout,
                                BoxTreeLifecycle::State::LayoutClean);
  DCHECK(!root_box->bounds().size().IsEmpty());
  LayoutVisitor().LayoutIfNeeded(root_box);
}

}  // namespace visuals
