// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <vector>

#include "evita/visuals/layout/layouter.h"

#include "base/logging.h"
#include "evita/visuals/geometry/float_rect.h"
#include "evita/visuals/model/block_box.h"
#include "evita/visuals/model/box_editor.h"
#include "evita/visuals/model/box_visitor.h"
#include "evita/visuals/model/line_box.h"

namespace visuals {

namespace {

//////////////////////////////////////////////////////////////////////
//
// LayoutVisitor
//
class LayoutVisitor final : public BoxVisitor {
 public:
  LayoutVisitor() = default;
  ~LayoutVisitor() final = default;

  void Layout(Box* box, const FloatRect& bounds);

 private:
#define V(name) void Visit##name(name* box) final;
  FOR_EACH_VISUAL_BOX(V)
#undef V

  DISALLOW_COPY_AND_ASSIGN(LayoutVisitor);
};

void LayoutVisitor::Layout(Box* box, const FloatRect& bounds) {
  if (box->bounds() == bounds && box->IsLayoutClean())
    return;
  BoxEditor().SetBounds(box, bounds);
  Visit(box);
  BoxEditor().SetLayoutClean(box);
}

// BoxVisitor
void LayoutVisitor::VisitBlockBox(BlockBox* box) {
  auto child_origin = FloatPoint();
  const auto child_width = box->content_bounds().width();
  for (const auto& child : box->child_boxes()) {
    if (child->is_display_none())
      continue;
    const auto& child_size = child->ComputePreferredSize();
    LayoutVisitor().Layout(
        child, FloatRect(child_origin + child->margin().top_left(),
                         FloatSize(child_width, child_size.height())));
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
    const auto& child_size = child->ComputePreferredSize();
    LayoutVisitor().Layout(
        child, FloatRect(child_origin + child->margin().top_left(),
                         FloatSize(child_size.width(), line_height)));
    child_origin = FloatPoint(child->bounds().right() + child->margin().right(),
                              child_origin.y());
  }
}

void LayoutVisitor::VisitTextBox(TextBox* box) {}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Layouter
//
Layouter::Layouter() {}

Layouter::~Layouter() {}

void Layouter::Layout(Box* box, const FloatRect& bounds) {
  LayoutVisitor().Layout(box, bounds);
}

}  // namespace visuals
