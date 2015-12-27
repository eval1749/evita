// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DISPLAY_PUBLIC_DISPLAY_ITEM_LIST_H_
#define EVITA_VISUALS_DISPLAY_PUBLIC_DISPLAY_ITEM_LIST_H_

#include <vector>

#include "base/macros.h"

namespace visuals {

class DisplayItem;
class DisplayItemListBuilder;

//////////////////////////////////////////////////////////////////////
//
// DisplayItemList
//
class DisplayItemList final {
 public:
  DisplayItemList();
  ~DisplayItemList();

  const std::vector<DisplayItem*>& items() const { return items_; }

 private:
  friend class DisplayItemListBuilder;

  std::vector<DisplayItem*> items_;

  DISALLOW_COPY_AND_ASSIGN(DisplayItemList);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_DISPLAY_PUBLIC_DISPLAY_ITEM_LIST_H_
