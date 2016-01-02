// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "evita/visuals/layout/size_calculator.h"

#include "evita/visuals/fonts/text_format.h"
#include "evita/visuals/geometry/float_rect.h"
#include "evita/visuals/model/block_box.h"
#include "evita/visuals/model/box_editor.h"
#include "evita/visuals/model/box_visitor.h"
#include "evita/visuals/model/line_box.h"
#include "evita/visuals/model/root_box.h"
#include "evita/visuals/model/text_box.h"

namespace visuals {

namespace {
//////////////////////////////////////////////////////////////////////
//
// PreferredSizeVisitor
//
class PreferredSizeVisitor final : public BoxVisitor {
 public:
  PreferredSizeVisitor() = default;
  ~PreferredSizeVisitor() final = default;

  FloatSize ComputePreferredSize(const Box& box);

 private:
  void ReturnSize(const FloatSize& size);

#define V(name) void Visit##name(name* box) final;
  FOR_EACH_VISUAL_BOX(V)
#undef V

  FloatSize result_;

  DISALLOW_COPY_AND_ASSIGN(PreferredSizeVisitor);
};

FloatSize PreferredSizeVisitor::ComputePreferredSize(const Box& box) {
  if (box.is_display_none())
    return FloatSize();
  Visit(const_cast<Box*>(&box));
  return result_;
}

void PreferredSizeVisitor::ReturnSize(const FloatSize& size) {
  result_ = size;
}

// BoxVisitor
void PreferredSizeVisitor::VisitBlockBox(BlockBox* box) {
  auto size = box->border().size() + box->padding().size();
  for (const auto& child : box->child_boxes()) {
    if (!child->position().is_static())
      continue;
    const auto& child_size = ComputePreferredSize(*child);
    size = FloatSize(std::max(size.width(), child_size.width()),
                     size.height() + child_size.height());
  }
  ReturnSize(size);
}

void PreferredSizeVisitor::VisitLineBox(LineBox* box) {
  auto size = box->border().size() + box->padding().size();
  for (const auto& child : box->child_boxes()) {
    if (!child->position().is_static())
      continue;
    const auto& child_size = ComputePreferredSize(*child);
    size = FloatSize(size.width() + child_size.width(),
                     std::max(size.height(), child_size.height()));
  }
  ReturnSize(size);
}

void PreferredSizeVisitor::VisitRootBox(RootBox* box) {
  ComputePreferredSize(*box->first_child());
}

void PreferredSizeVisitor::VisitTextBox(TextBox* box) {
  if (box->text().empty())
    return ReturnSize(FloatSize());
  // TODO(eval1749): Implement computing preferred size for |TextBox|.
  const auto font_size = 15.0f;
  const auto& text_format = std::make_unique<TextFormat>(L"Arial", font_size);
  const auto& width = text_format->ComputeWidth(box->text());
  ReturnSize(FloatSize(width, font_size) + box->border().size() +
             box->padding().size());
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// SizeCalculator
//
SizeCalculator::SizeCalculator() {}
SizeCalculator::~SizeCalculator() {}

FloatSize SizeCalculator::ComputePreferredSize(const Box& box) const {
  return PreferredSizeVisitor().ComputePreferredSize(box);
}

}  // namespace visuals
