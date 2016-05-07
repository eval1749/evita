// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "evita/visuals/display/public/display_items.h"
#include "evita/visuals/fonts/font_description_builder.h"
#include "evita/visuals/fonts/text_format_factory.h"
#include "evita/visuals/fonts/text_layout.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace visuals {

namespace {

std::unique_ptr<DrawTextDisplayItem> CreateDrawText(
    const gfx::FloatRect& bounds,
    const gfx::FloatColor& color,
    float baseline,
    const TextFormat& format,
    const base::string16& text) {
  return std::move(std::make_unique<DrawTextDisplayItem>(
      bounds, color, baseline, TextLayout(format, text, bounds.size()), text));
}

}  // namespace

TEST(BeginClipDisplayItemsTest, EqualsTo) {
  const auto& item1 = std::make_unique<BeginClipDisplayItem>(
      gfx::FloatRect(gfx::FloatSize(1, 2)));
  const auto& item2 = std::make_unique<BeginClipDisplayItem>(
      gfx::FloatRect(gfx::FloatSize(1, 2)));
  const auto& item3 = std::make_unique<BeginClipDisplayItem>(
      gfx::FloatRect(gfx::FloatSize(3, 4)));

  EXPECT_TRUE(*item1 == *item1);
  EXPECT_TRUE(*item2 == *item2);

  EXPECT_TRUE(*item1 == *item2);
  EXPECT_TRUE(*item2 == *item1);

  EXPECT_FALSE(*item1 != *item2);
  EXPECT_FALSE(*item2 != *item1);

  EXPECT_FALSE(*item1 == *item3);
  EXPECT_FALSE(*item3 == *item1);
  EXPECT_TRUE(*item1 != *item3);
  EXPECT_TRUE(*item3 != *item1);
}

TEST(DrawRectDisplayItemsTest, EqualsTo) {
  const auto& item1 = std::make_unique<DrawRectDisplayItem>(
      gfx::FloatRect(gfx::FloatSize(1, 2)), gfx::FloatColor(1, 1, 1), 1);
  const auto& item2 = std::make_unique<DrawRectDisplayItem>(
      gfx::FloatRect(gfx::FloatSize(1, 2)), gfx::FloatColor(1, 1, 1), 1);
  const auto& item3 = std::make_unique<DrawRectDisplayItem>(
      gfx::FloatRect(gfx::FloatSize(3, 4)), gfx::FloatColor(1, 1, 1), 1);
  const auto& item4 = std::make_unique<DrawRectDisplayItem>(
      gfx::FloatRect(gfx::FloatSize(1, 2)), gfx::FloatColor(0.5, 1, 1), 1);
  const auto& item5 = std::make_unique<DrawRectDisplayItem>(
      gfx::FloatRect(gfx::FloatSize(1, 2)), gfx::FloatColor(1, 1, 1), 2);

  EXPECT_TRUE(*item1 == *item1);
  EXPECT_TRUE(*item1 == *item2);
  EXPECT_FALSE(*item1 == *item3);
  EXPECT_FALSE(*item1 == *item4);
  EXPECT_FALSE(*item1 == *item5);

  EXPECT_FALSE(*item1 != *item1);
  EXPECT_FALSE(*item1 != *item2);
  EXPECT_TRUE(*item1 != *item3);
  EXPECT_TRUE(*item1 != *item4);
  EXPECT_TRUE(*item1 != *item5);

  EXPECT_TRUE(*item2 == *item1);

  EXPECT_FALSE(*item3 == *item1);
  EXPECT_TRUE(*item3 != *item1);

  EXPECT_FALSE(*item4 == *item1);
  EXPECT_TRUE(*item4 != *item1);

  EXPECT_FALSE(*item5 == *item1);
  EXPECT_TRUE(*item5 != *item1);
}

TEST(DrawTextDisplayItemsTest, EqualsTo) {
  const auto& font =
      FontDescription::Builder().SetFamily(L"Arial").SetSize(10).Build();
  const auto& format = TextFormatFactory::GetInstance()->Get(font);
  const auto& item1 =
      CreateDrawText(gfx::FloatRect(gfx::FloatSize(1, 2)),
                     gfx::FloatColor(1, 1, 1), 1, format, L"foo");
  const auto& item2 = item1;
  const auto& item3 =
      CreateDrawText(gfx::FloatRect(gfx::FloatSize(3, 4)),
                     gfx::FloatColor(1, 1, 1), 1, format, L"foo");
  const auto& item4 =
      CreateDrawText(gfx::FloatRect(gfx::FloatSize(1, 2)),
                     gfx::FloatColor(0.5, 1, 1), 1, format, L"foo");
  const auto& item5 =
      CreateDrawText(gfx::FloatRect(gfx::FloatSize(1, 2)),
                     gfx::FloatColor(1, 1, 1), 2, format, L"foo");
  const auto& item6 =
      CreateDrawText(gfx::FloatRect(gfx::FloatSize(1, 2)),
                     gfx::FloatColor(1, 1, 1), 1, format, L"bar");

  EXPECT_TRUE(*item1 == *item1);
  EXPECT_TRUE(*item1 == *item2);
  EXPECT_FALSE(*item1 == *item3);
  EXPECT_FALSE(*item1 == *item4);
  EXPECT_FALSE(*item1 == *item5);
  EXPECT_FALSE(*item1 == *item6);

  EXPECT_FALSE(*item1 != *item1);
  EXPECT_FALSE(*item1 != *item2);
  EXPECT_TRUE(*item1 != *item3);
  EXPECT_TRUE(*item1 != *item4);
  EXPECT_TRUE(*item1 != *item5);
  EXPECT_TRUE(*item1 != *item6);

  EXPECT_TRUE(*item2 == *item1);

  EXPECT_FALSE(*item3 == *item1);
  EXPECT_TRUE(*item3 != *item1);

  EXPECT_FALSE(*item4 == *item1);
  EXPECT_TRUE(*item4 != *item1);

  EXPECT_FALSE(*item5 == *item1);
  EXPECT_TRUE(*item5 != *item1);

  EXPECT_FALSE(*item6 == *item1);
  EXPECT_TRUE(*item6 != *item1);
}

TEST(EndClipDisplayItemsTest, EqualsTo) {
  const auto& item1 = std::make_unique<EndClipDisplayItem>();
  const auto& item2 = std::make_unique<EndClipDisplayItem>();

  EXPECT_TRUE(*item1 == *item1);
  EXPECT_TRUE(*item1 == *item2);

  EXPECT_FALSE(*item1 != *item1);
  EXPECT_FALSE(*item1 != *item2);

  EXPECT_TRUE(*item2 == *item1);
  EXPECT_FALSE(*item2 != *item1);
}

TEST(FillRectDisplayItemsTest, EqualsTo) {
  const auto& item1 = std::make_unique<FillRectDisplayItem>(
      gfx::FloatRect(gfx::FloatSize(1, 2)), gfx::FloatColor(1, 1, 1));
  const auto& item2 = std::make_unique<FillRectDisplayItem>(
      gfx::FloatRect(gfx::FloatSize(1, 2)), gfx::FloatColor(1, 1, 1));
  const auto& item3 = std::make_unique<FillRectDisplayItem>(
      gfx::FloatRect(gfx::FloatSize(1, 2)), gfx::FloatColor(0.5, 1, 1));
  const auto& item4 = std::make_unique<FillRectDisplayItem>(
      gfx::FloatRect(gfx::FloatSize(2, 2)), gfx::FloatColor(1, 1, 1));

  EXPECT_TRUE(*item1 == *item1);
  EXPECT_TRUE(*item1 == *item2);
  EXPECT_FALSE(*item1 == *item3);
  EXPECT_FALSE(*item1 == *item4);

  EXPECT_FALSE(*item1 != *item1);
  EXPECT_FALSE(*item1 != *item2);
  EXPECT_TRUE(*item1 != *item3);
  EXPECT_TRUE(*item1 != *item4);

  EXPECT_TRUE(*item2 == *item1);

  EXPECT_FALSE(*item3 == *item1);
  EXPECT_TRUE(*item3 != *item1);

  EXPECT_FALSE(*item4 == *item1);
  EXPECT_TRUE(*item4 != *item1);
}

}  // namespace visuals
