// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <vector>

#include "evita/editor/dom_lock.h"
#include "evita/layout/block_flow.h"
#include "evita/text/buffer.h"
#include "gtest/gtest.h"

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
class BlockFlowTest : public ::testing::Test {
 protected:
  BlockFlowTest();
  ~BlockFlowTest() override;

  BlockFlow* block() const { return block_.get(); }
  text::Buffer* buffer() const { return buffer_.get(); }

  text::Offset HitTestPoint(const gfx::PointF& view_point) const;

 private:
  const std::unique_ptr<text::Buffer> buffer_;

  // |BlockFlow| constructor takes |buffer_|.
  const std::unique_ptr<BlockFlow> block_;

  DISALLOW_COPY_AND_ASSIGN(BlockFlowTest);
};

BlockFlowTest::BlockFlowTest()
    : buffer_(new text::Buffer()), block_(new BlockFlow(*buffer_)) {
  block()->SetBounds(
      gfx::RectF(gfx::PointF(300.0f, 200.0f), gfx::SizeF(100.0f, 150.0f)));
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
  EXPECT_EQ(text::Offset(2), HitTestPoint(gfx::PointF(331, 200)));
  EXPECT_EQ(text::Offset(4), HitTestPoint(gfx::PointF(310, 215)));
  EXPECT_EQ(text::Offset(5), HitTestPoint(gfx::PointF(318, 215)));

  // Points outsize block
  EXPECT_EQ(text::Offset(0), HitTestPoint(gfx::PointF(0, 0)));
  EXPECT_EQ(text::Offset(13), HitTestPoint(gfx::PointF(999, 999)));
}

TEST_F(BlockFlowTest, HitTestTextPosition) {
  buffer()->InsertBefore(text::Offset(0), L"foo\nbar\nbarz\n");
  block()->Format(text::Offset(0));

  EXPECT_EQ(CaretBoundsOf(310, 200, 15),
            block()->HitTestTextPosition(text::Offset(0)));
  EXPECT_EQ(CaretBoundsOf(318, 200, 15),
            block()->HitTestTextPosition(text::Offset(1)));
  EXPECT_EQ(CaretBoundsOf(331, 200, 15),
            block()->HitTestTextPosition(text::Offset(3)));
  EXPECT_EQ(CaretBoundsOf(310, 215, 15),
            block()->HitTestTextPosition(text::Offset(4)));
  EXPECT_EQ(CaretBoundsOf(318, 215, 15),
            block()->HitTestTextPosition(text::Offset(5)));
}

}  // namespace layout
