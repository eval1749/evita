// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/layout/box.h"

#include "base/logging.h"
#include "evita/visuals/dom/node.h"
#include "evita/visuals/layout/ancestors_or_self.h"
#include "evita/visuals/layout/border.h"
#include "evita/visuals/layout/container_box.h"
#include "evita/visuals/layout/margin.h"
#include "evita/visuals/layout/padding.h"
#include "evita/visuals/layout/root_box.h"

namespace visuals {

namespace {
int last_box_sequence_id;
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Box
//
Box::Box(RootBox* root_box, const Node* node)
    : node_(node), sequence_id_(++last_box_sequence_id), root_box_(root_box) {}

Box::Box(RootBox* root_box) : Box(root_box, nullptr) {}

Box::~Box() {
  DCHECK_EQ(static_cast<ContainerBox*>(nullptr), parent_);
  DCHECK_EQ(static_cast<Box*>(nullptr), next_sibling_);
  DCHECK_EQ(static_cast<Box*>(nullptr), previous_sibling_);
}

FloatRect Box::content_bounds() const {
  const auto& border = ComputeBorder();
  const auto& padding = ComputePadding();
  return FloatRect(FloatPoint() + border.top_left() + padding.top_left(),
                   bounds_.size() - border.top_left() - padding.top_left() -
                       border.bottom_right() - padding.bottom_right());
}

Border Box::ComputeBorder() const {
  return Border(border_top_color_, border_top_width_.value(),
                border_right_color_, border_right_width_.value(),
                border_bottom_color_, border_bottom_width_.value(),
                border_left_color_, border_left_width_.value());
}

Margin Box::ComputeMargin() const {
  return Margin(margin_top_.value(), margin_right_.value(),
                margin_bottom_.value(), margin_left_.value());
}

Padding Box::ComputePadding() const {
  return Padding(padding_top_.value(), padding_right_.value(),
                 padding_bottom_.value(), padding_left_.value());
}

void Box::DidChangeBounds(const FloatRect& old_bounds) {
  DCHECK(root_box_->InLayout()) << root_box_->lifecycle();
}

bool Box::InDocument() const {
  for (const auto& runner : Box::AncestorsOrSelf(*this)) {
    if (const auto root_box = runner->as<RootBox>())
      return true;
  }
  return false;
}

bool Box::IsDescendantOf(const Box& other) const {
  for (auto runner = parent(); runner; runner = runner->parent()) {
    if (runner == &other)
      return true;
  }
  return false;
}

std::ostream& operator<<(std::ostream& ostream, const Box& box) {
  ostream << box.class_name() << '.' << box.sequence_id();
  if (box.node())
    ostream << ' ' << *box.node();
  else
    ostream << "#anonymous";
  return ostream << ' ' << box.bounds();
}

std::ostream& operator<<(std::ostream& ostream, const Box* box) {
  if (!box)
    return ostream << "nullptr";
  return ostream << *box;
}

}  // namespace visuals
