// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/display/public/display_item_list.h"

#include "evita/visuals/display/public/display_items.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// DisplayItemList
//
DisplayItemList::DisplayItemList() {}

DisplayItemList::~DisplayItemList() {
  for (const auto& item : items_)
    delete item;
}

}  // namespace visuals
