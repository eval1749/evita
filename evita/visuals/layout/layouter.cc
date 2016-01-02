// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <vector>

#include "evita/visuals/layout/layouter.h"

#include "base/logging.h"
#include "evita/visuals/geometry/float_rect.h"
#include "evita/visuals/layout/size_calculator.h"
#include "evita/visuals/model/block_box.h"
#include "evita/visuals/model/box_editor.h"
#include "evita/visuals/model/box_visitor.h"
#include "evita/visuals/model/line_box.h"
#include "evita/visuals/model/root_box.h"

namespace visuals {

namespace {

bool NeedsLayout(const Box& box, const FloatRect& bounds) {
  if (box.bounds() != bounds)
    return true;
  const auto container = box.as<ContainerBox>();
  if (!container)
    return false;
  return container->IsChildrenChanged();
}

//////////////////////////////////////////////////////////////////////
//
// LayoutVisitor
//
class LayoutVisitor final : public BoxVisitor {
 public:
  LayoutVisitor() = default;
  ~LayoutVisitor() final = default;

  void Layout(Box* box, const FloatPoint& origin, const FloatSize& size);
  void Layout(Box* box, const FloatRect& bounds);

 private:
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
  if (NeedsLayout(*box, bounds)) {
    BoxEditor().SetBounds(box, bounds);
    Visit(box);
    BoxEditor().DidLayout(box);
    return;
  }

  const auto container = box->as<ContainerBox>();
  if (!container || !container->IsSubtreeChanged())
    return;
  for (const auto& child : container->child_boxes())
    Visit(child);
}

// BoxVisitor
void LayoutVisitor::VisitBlockBox(BlockBox* box) {
  auto child_origin = FloatPoint();
  const auto content_width = box->content_bounds().width();
  for (const auto& child : box->child_boxes()) {
    if (child->is_display_none())
      continue;
    const auto& child_size = SizeCalculator().ComputePreferredSize(*child);
    if (child->position().is_absolute()) {
      LayoutVisitor().Layout(child, FloatPoint(child->left().length().value(),
                                               child->top().length().value()),
                             FloatSize(content_width, child_size.height()));
      continue;
    }
    LayoutVisitor().Layout(
        child, FloatRect(child_origin + child->margin().top_left(),
                         FloatSize(content_width, child_size.height())));
    child_origin = FloatPoint(
        child_origin.x(), child->bounds().bottom() + child->margin().bottom());
  }
}

void LayoutVisitor::VisitLineBox(LineBox* line) {
  auto child_origin = FloatPoint();
  const auto line_height = line->content_bounds().height();
  for (const auto& child : line->child_boxes()) {
    if (child->is_display_none())
      continue;
    const auto& child_size = SizeCalculator().ComputePreferredSize(*child);
    LayoutVisitor().Layout(
        child, FloatRect(child_origin + child->margin().top_left(),
                         FloatSize(child_size.width(), line_height)));
    child_origin = FloatPoint(child->bounds().right() + child->margin().right(),
                              child_origin.y());
  }
}

void LayoutVisitor::VisitRootBox(RootBox* root) {
  LayoutVisitor().Layout(root->first_child(), root->content_bounds());
}

void LayoutVisitor::VisitTextBox(TextBox* box) {}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Layouter
//
Layouter::Layouter() {}

Layouter::~Layouter() {}

void Layouter::Layout(RootBox* root_box) {
  if (root_box->IsLayoutClean())
    return;
  BoxTreeLifecycle::Scope scope(root_box->lifecycle(),
                                BoxTreeLifecycle::State::InLayout,
                                BoxTreeLifecycle::State::LayoutClean);
  LayoutVisitor().Layout(root_box, FloatRect(root_box->viewport_size()));
}

}  // namespace visuals
