// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DISPLAY_DISPLAY_ITEM_LIST_BUILDER_H_
#define EVITA_VISUALS_DISPLAY_DISPLAY_ITEM_LIST_BUILDER_H_

#include <memory>
#include <vector>

#include "base/macros.h"
#include "evita/visuals/geometry/float_rect.h"

namespace visuals {

class DisplayItem;
class DisplayItemList;

//////////////////////////////////////////////////////////////////////
//
// DisplayItemListBuilder
//
class DisplayItemListBuilder final {
 public:
  explicit DisplayItemListBuilder(const FloatRect& viewport_bounds);
  ~DisplayItemListBuilder();

  template <typename T, typename... Args>
  void AddNew(Args&&... args) {
    AddItem(std::move(
        std::unique_ptr<DisplayItem>(new T(std::forward<Args>(args)...))));
  }

  void AddRect(const FloatRect& rect);
  std::unique_ptr<DisplayItemList> Build();

 private:
  void AddItem(std::unique_ptr<DisplayItem> item);

  std::unique_ptr<DisplayItemList> list_;
  std::vector<FloatRect> rects_;

  // Display item list should contains items paint inside |viewport_bounds_|.
  const FloatRect viewport_bounds_;

  DISALLOW_COPY_AND_ASSIGN(DisplayItemListBuilder);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_DISPLAY_DISPLAY_ITEM_LIST_BUILDER_H_
