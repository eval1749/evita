// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/display/display_item_list_builder.h"
#include "evita/visuals/display/public/display_items.h"
#include "evita/visuals/display/public/display_item_list.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(DisplayItemList, Basic) {
  DisplayItemListBuilder builder;
  builder.AddNew<BeginBoxDisplayItem>(1, FloatRect(FloatSize(1, 2)));
  builder.AddNew<FillRectDisplayItem>(FloatRect(FloatSize(1, 2)),
                                      FloatColor(1, 1, 1));
  builder.AddNew<EndBoxDisplayItem>(1);
  const auto& list = builder.Build();

  EXPECT_EQ(3, list->items().size());
}

}  // namespace visuals
