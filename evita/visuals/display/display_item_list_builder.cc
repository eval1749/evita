// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/display/display_item_list_builder.h"

#include "base/logging.h"
#include "evita/gfx/base/geometry/float_rect.h"
#include "evita/visuals/display/public/display_item_list.h"
#include "evita/visuals/display/public/display_items.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// DisplayItemListBuilder
//
DisplayItemListBuilder::DisplayItemListBuilder(
    const gfx::FloatRect& viewport_bounds)
    : list_(new DisplayItemList()), viewport_bounds_(viewport_bounds) {}

DisplayItemListBuilder::~DisplayItemListBuilder() {
  DCHECK(!list_);
}

void DisplayItemListBuilder::AddItem(std::unique_ptr<DisplayItem> item) {
  list_->items_.push_back(item.release());
}

void DisplayItemListBuilder::AddRect(const gfx::FloatRect& rect) {
  const auto& clipped = viewport_bounds_.Intersect(rect);
  if (clipped.IsEmpty())
    return;
  if (!list_->rects_.empty() && list_->rects_.front().Contains(clipped))
    return;
  list_->rects_.push_back(clipped);
}

std::unique_ptr<DisplayItemList> DisplayItemListBuilder::Build() {
  DCHECK(list_);
  return std::move(list_);
}

}  // namespace visuals
