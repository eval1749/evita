// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/display/display_item_list_builder.h"

#include "base/logging.h"
#include "evita/visuals/display/public/display_items.h"
#include "evita/visuals/display/public/display_item_list.h"
#include "evita/visuals/geometry/float_rect.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// DisplayItemListBuilder
//
DisplayItemListBuilder::DisplayItemListBuilder()
    : list_(new DisplayItemList()) {}

DisplayItemListBuilder::~DisplayItemListBuilder() {
  DCHECK(!list_);
}

void DisplayItemListBuilder::Add(std::unique_ptr<DisplayItem> item) {
  list_->items_.push_back(item.release());
}

std::unique_ptr<DisplayItemList> DisplayItemListBuilder::Build() {
  DCHECK(list_);
  return std::move(list_);
}

}  // namespace visuals
