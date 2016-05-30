// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "evita/text/layout/text_layout_test_base.h"

#include "evita/css/style_sheet.h"
#include "evita/editor/dom_lock.h"
#include "evita/text/layout/block_flow.h"
#include "evita/text/models/buffer.h"
#include "evita/text/models/marker_set.h"
#include "evita/text/style/style_tree.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace layout {

namespace {

gfx::RectF CaretBoundsOf(int origin_x, int origin_y, int height) {
  return gfx::RectF(
      gfx::PointF(static_cast<float>(origin_x), static_cast<float>(origin_y)),
      gfx::SizeF(1.0f, static_cast<float>(height)));
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// BlockFlowTest
//
class BlockFlowTest : public TextLayoutTestBase {
 protected:
  BlockFlowTest();
  ~BlockFlowTest() override;

  BlockFlow* block() const { return block_.get(); }

  text::Offset HitTestPoint(const gfx::PointF& view_point) const;

 private:
  const std::unique_ptr<BlockFlow> block_;

  DISALLOW_COPY_AND_ASSIGN(BlockFlowTest);
};

BlockFlowTest::BlockFlowTest()
    : block_(new BlockFlow(*buffer(), *markers(), style_tree())) {
  set_bounds(
      gfx::RectF(gfx::PointF(300.0f, 200.0f), gfx::SizeF(100.0f, 150.0f)));
  block()->SetBounds(bounds());
  editor::DomLock::GetInstance()->Acquire(FROM_HERE);
}

BlockFlowTest::~BlockFlowTest() {
  editor::DomLock::GetInstance()->Release(FROM_HERE);
}

text::Offset BlockFlowTest::HitTestPoint(const gfx::PointF& view_point) const {
  const auto& block_point = view_point - block()->origin();
  return block()->HitTestPoint(block_point);
}

TEST_F(BlockFlowTest, HitTestPoint) {
  buffer()->InsertBefore(text::Offset(0), L"foo\nbar\nbarz\n");
  block()->Format(text::Offset(0));

  EXPECT_EQ(text::Offset(0), HitTestPoint(gfx::PointF(310, 200)));
  EXPECT_EQ(text::Offset(1), HitTestPoint(gfx::PointF(318, 200)));
  EXPECT_EQ(text::Offset(2), HitTestPoint(gfx::PointF(328, 200)));
  EXPECT_EQ(text::Offset(4), HitTestPoint(gfx::PointF(310, 220)));
  EXPECT_EQ(text::Offset(5), HitTestPoint(gfx::PointF(318, 220)));

  // Points outsize block
  EXPECT_EQ(text::Offset(0), HitTestPoint(gfx::PointF(0, 0)));
  EXPECT_EQ(text::Offset(13), HitTestPoint(gfx::PointF(999, 999)));
}

TEST_F(BlockFlowTest, HitTestTextPosition) {
  buffer()->InsertBefore(text::Offset(0), L"foo\nbar\nbarz\n");
  block()->Format(text::Offset(0));

  EXPECT_EQ(CaretBoundsOf(310, 200, 16),
            block()->HitTestTextPosition(text::Offset(0)));
  EXPECT_EQ(CaretBoundsOf(317, 200, 16),
            block()->HitTestTextPosition(text::Offset(1)));
  EXPECT_EQ(CaretBoundsOf(331, 200, 16),
            block()->HitTestTextPosition(text::Offset(3)));
  EXPECT_EQ(CaretBoundsOf(310, 216, 16),
            block()->HitTestTextPosition(text::Offset(4)));
  EXPECT_EQ(CaretBoundsOf(317, 216, 16),
            block()->HitTestTextPosition(text::Offset(5)));
}

}  // namespace layout
