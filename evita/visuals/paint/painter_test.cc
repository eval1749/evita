// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/display/display_item_list_builder.h"
#include "evita/visuals/display/public/display_items.h"
#include "evita/visuals/display/public/display_item_list.h"
#include "evita/visuals/geometry/float_rect.h"
#include "evita/visuals/layout/layouter.h"
#include "evita/visuals/model/block_box.h"
#include "evita/visuals/model/box_builder.h"
#include "evita/visuals/paint/painter.h"
#include "evita/visuals/style/float_color.h"
#include "evita/visuals/style/style.h"
#include "evita/visuals/style/style_builder.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(PainterTest, Basic) {
  const auto& root_box =
      BoxBuilder::New<BlockBox>()
          .SetStyle(*StyleBuilder()
                         .SetBackground(Background(FloatColor(1, 1, 1)))
                         .Build())
          .Finish();
  Layouter().Layout(root_box.get(), FloatRect(FloatSize(200, 100)));
  const auto& display_item_list = Painter().Paint(*root_box);
  EXPECT_EQ(3, display_item_list->items().size());
}

}  // namespace visuals
