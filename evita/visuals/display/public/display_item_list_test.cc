// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/display/public/display_item_list.h"
#include "evita/visuals/display/display_item_list_builder.h"
#include "evita/visuals/display/public/display_items.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(DisplayItemList, Basic) {
  gfx::FloatRect viewport_bounds(gfx::FloatSize(10, 10));
  DisplayItemListBuilder builder(viewport_bounds);
  builder.AddNew<BeginClipDisplayItem>(gfx::FloatRect(gfx::FloatSize(1, 2)));
  builder.AddNew<FillRectDisplayItem>(gfx::FloatRect(gfx::FloatSize(1, 2)),
                                      gfx::FloatColor(1, 1, 1));
  builder.AddNew<EndClipDisplayItem>();
  const auto& list = builder.Build();

  EXPECT_EQ(3, list->items().size());
}

}  // namespace visuals
