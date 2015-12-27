// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DISPLAY_DISPLAY_ITEM_LIST_BUILDER_H_
#define EVITA_VISUALS_DISPLAY_DISPLAY_ITEM_LIST_BUILDER_H_

#include <memory>

#include "base/macros.h"

namespace visuals {

class DisplayItem;
class DisplayItemList;

//////////////////////////////////////////////////////////////////////
//
// DisplayItemListBuilder
//
class DisplayItemListBuilder final {
 public:
  DisplayItemListBuilder();
  ~DisplayItemListBuilder();

  template <typename T>
  void Add(std::unique_ptr<T>&& child) {
    Add(std::unique_ptr<DisplayItem>(child.release()));
  }

  void Add(std::unique_ptr<DisplayItem> item);
  std::unique_ptr<DisplayItemList> Build();

 private:
  std::unique_ptr<DisplayItemList> list_;

  DISALLOW_COPY_AND_ASSIGN(DisplayItemListBuilder);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_DISPLAY_DISPLAY_ITEM_LIST_BUILDER_H_
