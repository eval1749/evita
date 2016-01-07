// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/css/float_color.h"
#include "evita/visuals/css/style.h"
#include "evita/visuals/css/style_builder.h"
#include "evita/visuals/display/display_item_list_builder.h"
#include "evita/visuals/display/public/display_items.h"
#include "evita/visuals/display/public/display_item_list.h"
#include "evita/visuals/geometry/float_rect.h"
#include "evita/visuals/layout/box_editor.h"
#include "evita/visuals/layout/flow_box.h"
#include "evita/visuals/layout/layouter.h"
#include "evita/visuals/layout/root_box.h"
#include "evita/visuals/layout/simple_box_tree_builder.h"
#include "evita/visuals/paint/painter.h"
#include "evita/visuals/paint/paint_info.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(PainterTest, Basic) {
  const auto& root =
      SimpleBoxTreeBuilder()
          .Begin<FlowBox>()
          .SetStyle(*css::StyleBuilder()
                         .SetBackground(css::Background(css::Color(1, 1, 1)))
                         .Build())
          .End<FlowBox>()
          .Build();
  BoxEditor().SetViewportSize(root.get(), FloatSize(200, 100));
  Layouter().Layout(root.get());
  PaintInfo paint_info(FloatRect(root->viewport_size()));
  const auto& display_item_list = Painter().Paint(paint_info, *root);
  EXPECT_EQ(3, display_item_list->items().size());

  BoxEditor().RemoveDescendants(root.get());
}

}  // namespace visuals
