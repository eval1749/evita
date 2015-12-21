// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/marker_set.h"

#include "base/observer_list.h"
#include "evita/text/buffer.h"
#include "evita/text/marker.h"
#include "evita/text/static_range.h"
#include "gtest/gtest.h"

namespace text {

//////////////////////////////////////////////////////////////////////
//
// MarkerSetTest
//
class MarkerSetTest : public ::testing::Test {
 protected:
  MarkerSetTest();
  ~MarkerSetTest() override = default;

  Buffer* buffer() const { return buffer_.get(); }
  MarkerSet* marker_set() { return &marker_set_; }

  Marker GetAt(int offset) {
    auto const marker = marker_set_.GetMarkerAt(Offset(offset));
    return marker ? *marker : Marker();
  }

  void InsertMarker(int start, int end, const common::AtomicString& type);
  void RemoveMarker(int start, int end);

  const common::AtomicString Correct;
  const common::AtomicString Misspelled;

 private:
  const std::unique_ptr<Buffer> buffer_;
  MarkerSet marker_set_;

  DISALLOW_COPY_AND_ASSIGN(MarkerSetTest);
};

MarkerSetTest::MarkerSetTest()
    : Correct(L"Correct"),
      Misspelled(L"Misspelled"),
      buffer_(new Buffer()),
      marker_set_(buffer_.get()) {
  buffer_->InsertBefore(Offset(0), base::string16(999, 'x'));
}

void MarkerSetTest::InsertMarker(int start,
                                 int end,
                                 const common::AtomicString& type) {
  marker_set()->InsertMarker(StaticRange(*buffer_, Offset(start), Offset(end)),
                             type);
}

void MarkerSetTest::RemoveMarker(int start, int end) {
  marker_set()->RemoveMarkerForTesting(
      StaticRange(*buffer_, Offset(start), Offset(end)));
}

TEST_F(MarkerSetTest, DeleteMarker_cover) {
  // before: --CC--
  // insert: -____--
  // after:  -____--
  InsertMarker(150, 250, Correct);
  RemoveMarker(100, 300);
  EXPECT_EQ(Marker(), GetAt(100));
  EXPECT_EQ(Marker(), GetAt(125));
  EXPECT_EQ(Marker(), GetAt(150));
  EXPECT_EQ(Marker(), GetAt(225));
  EXPECT_EQ(Marker(), GetAt(250));
}

TEST_F(MarkerSetTest, DeleteMarker_cross_left) {
  // before: ---CC--
  // insert: --__---
  // after:  --__C--
  InsertMarker(200, 300, Correct);
  RemoveMarker(150, 250);
  EXPECT_EQ(Marker(), GetAt(150));
  EXPECT_EQ(Marker(Offset(250), Offset(300), Correct), GetAt(250));
}

TEST_F(MarkerSetTest, DeleteMarker_cross_right) {
  // before: --CCCC----
  // insert: ----____--
  // after:  --CC__----
  InsertMarker(100, 250, Correct);
  RemoveMarker(200, 300);
  EXPECT_EQ(Marker(Offset(100), Offset(200), Correct), GetAt(150));
  EXPECT_EQ(Marker(), GetAt(200));
  EXPECT_EQ(Marker(), GetAt(250));
}

TEST_F(MarkerSetTest, DeleteMarker_disjoint) {
  // before: --CC--
  // insert: ------__--
  // after:  --CC------
  InsertMarker(100, 200, Correct);
  RemoveMarker(300, 400);
  EXPECT_EQ(Marker(Offset(100), Offset(200), Correct), GetAt(150));
  EXPECT_EQ(Marker(), GetAt(350));
}

TEST_F(MarkerSetTest, DeleteMarker_same) {
  InsertMarker(100, 200, Correct);
  RemoveMarker(100, 200);
  auto const marker = marker_set()->GetLowerBoundMarker(Offset(100));
  EXPECT_EQ(Marker(), marker ? *marker : Marker());
}

TEST_F(MarkerSetTest, DeleteMarker_split) {
  // before: -CCCCCC--
  // insert: ---__--

  // after:  -CC__CC--
  InsertMarker(200, 400, Correct);
  RemoveMarker(250, 350);
  EXPECT_EQ(Marker(), GetAt(199));
  EXPECT_EQ(Marker(Offset(200), Offset(250), Correct), GetAt(200));
  EXPECT_EQ(Marker(Offset(200), Offset(250), Correct), GetAt(225));
  EXPECT_EQ(Marker(), GetAt(300));
  EXPECT_EQ(Marker(), GetAt(349));
  EXPECT_EQ(Marker(Offset(350), Offset(400), Correct), GetAt(350));
  EXPECT_EQ(Marker(Offset(350), Offset(400), Correct), GetAt(399));
  EXPECT_EQ(Marker(), GetAt(400));
}

TEST_F(MarkerSetTest, DidDeleteAt) {
  // before: --"foo"x--
  // delete: --"foo"---
  InsertMarker(0, 5, Correct);
  InsertMarker(5, 6, Misspelled);
  buffer()->Delete(Offset(5), Offset(6));
  EXPECT_EQ(Marker(Offset(0), Offset(5), Correct), GetAt(0));
}

TEST_F(MarkerSetTest, GetMarkerAt) {
  InsertMarker(5, 10, Correct);
  EXPECT_EQ(Marker(), GetAt(0));
  EXPECT_EQ(Marker(), GetAt(4));
  EXPECT_EQ(Marker(Offset(5), Offset(10), Correct), GetAt(5));
  EXPECT_EQ(Marker(Offset(5), Offset(10), Correct), GetAt(9));
  EXPECT_EQ(Marker(), GetAt(10));
  EXPECT_EQ(Marker(), GetAt(11));
}

TEST_F(MarkerSetTest, GetLowerBoundMarker) {
  InsertMarker(5, 10, Correct);
  EXPECT_EQ(Marker(), GetAt(0));
  EXPECT_EQ(Marker(), GetAt(4));
  EXPECT_EQ(Marker(Offset(5), Offset(10), Correct), GetAt(5));
  EXPECT_EQ(Marker(Offset(5), Offset(10), Correct), GetAt(9));
  EXPECT_EQ(Marker(), GetAt(10));
}

TEST_F(MarkerSetTest, InsertMarker_cover) {
  // before: --CC--
  // insert: -MMMM--
  // after:  -MMMM--
  InsertMarker(150, 250, Correct);
  InsertMarker(100, 300, Misspelled);
  EXPECT_EQ(Marker(Offset(100), Offset(300), Misspelled), GetAt(100));
  EXPECT_EQ(Marker(Offset(100), Offset(300), Misspelled), GetAt(125));
  EXPECT_EQ(Marker(Offset(100), Offset(300), Misspelled), GetAt(150));
  EXPECT_EQ(Marker(Offset(100), Offset(300), Misspelled), GetAt(225));
  EXPECT_EQ(Marker(Offset(100), Offset(300), Misspelled), GetAt(250));
}

TEST_F(MarkerSetTest, InsertMarker_cross_left) {
  // before: ---MM--
  // insert: --CC---
  // after:  --CCM--
  InsertMarker(200, 300, Correct);
  InsertMarker(150, 250, Misspelled);
  EXPECT_EQ(Marker(Offset(150), Offset(250), Misspelled), GetAt(150));
  EXPECT_EQ(Marker(Offset(250), Offset(300), Correct), GetAt(250));
}

TEST_F(MarkerSetTest, InsertMarker_cross_right) {
  // before: --MMMM----
  // insert: ----CCCC--
  // after:  --MMCCCC--
  InsertMarker(100, 250, Misspelled);
  InsertMarker(200, 300, Correct);
  EXPECT_EQ(Marker(Offset(100), Offset(200), Misspelled), GetAt(150));
  EXPECT_EQ(Marker(Offset(200), Offset(300), Correct), GetAt(200));
  EXPECT_EQ(Marker(Offset(200), Offset(300), Correct), GetAt(250));
}

TEST_F(MarkerSetTest, InsertMarker_disjoint) {
  // before: --CC--
  // insert: ------CC--
  // after:  --CC--CC--
  InsertMarker(100, 200, Correct);
  InsertMarker(300, 400, Correct);
  EXPECT_EQ(Marker(Offset(100), Offset(200), Correct), GetAt(150));
  EXPECT_EQ(Marker(Offset(300), Offset(400), Correct), GetAt(350));
}

TEST_F(MarkerSetTest, InsertMarker_existing) {
  // before: --CCCCCCC--
  // insert: ----CCCCC--
  // after:  --CCCCCCC--
  InsertMarker(10, 30, Correct);
  InsertMarker(20, 30, Correct);
  EXPECT_EQ(Marker(Offset(10), Offset(30), Correct), GetAt(10));
}

TEST_F(MarkerSetTest, InsertMarker_extend) {
  // before: --CCCCCCC----
  // insert: ----CCCCCCC--
  // after:  --CCCCCCCCC--
  InsertMarker(10, 30, Correct);
  InsertMarker(20, 40, Correct);
  EXPECT_EQ(Marker(Offset(10), Offset(40), Correct), GetAt(10));
}

TEST_F(MarkerSetTest, InsertMarker_merge) {
  // before: --CC--CC--CC--
  // insert: --MMMMMMMMMM--
  // after:  --MMMMMMMMMM--
  InsertMarker(10, 20, Correct);
  InsertMarker(40, 60, Correct);
  InsertMarker(80, 90, Correct);
  InsertMarker(10, 90, Misspelled);
  EXPECT_EQ(Marker(Offset(10), Offset(90), Misspelled), GetAt(10));
  EXPECT_EQ(Marker(Offset(10), Offset(90), Misspelled), GetAt(20));
  EXPECT_EQ(Marker(Offset(10), Offset(90), Misspelled), GetAt(30));
  EXPECT_EQ(Marker(Offset(10), Offset(90), Misspelled), GetAt(89));
  EXPECT_EQ(Marker(), GetAt(90));
}

TEST_F(MarkerSetTest, InsertMarker_merge2) {
  // before:  --CCCC-CCCC--
  // insert: ------C------
  // after:  --CCCCCCCC---
  InsertMarker(0, 4, Correct);
  InsertMarker(5, 9, Correct);
  InsertMarker(4, 5, Correct);
  EXPECT_EQ(Marker(Offset(0), Offset(9), Correct), GetAt(1));
}

TEST_F(MarkerSetTest, InsertMarker_split) {
  // before: --CCCCCC--
  // insert: ----MM----
  // after:  --CCMMCC--
  InsertMarker(200, 400, Correct);

  InsertMarker(250, 350, Misspelled);
  EXPECT_EQ(Marker(), GetAt(199));
  EXPECT_EQ(Marker(Offset(200), Offset(250), Correct), GetAt(200));
  EXPECT_EQ(Marker(Offset(200), Offset(250), Correct), GetAt(225));
  EXPECT_EQ(Marker(Offset(250), Offset(350), Misspelled), GetAt(300));
  EXPECT_EQ(Marker(Offset(250), Offset(350), Misspelled), GetAt(349));
  EXPECT_EQ(Marker(Offset(350), Offset(400), Correct), GetAt(350));
  EXPECT_EQ(Marker(Offset(350), Offset(400), Correct), GetAt(399));
  EXPECT_EQ(Marker(), GetAt(400));
}

}  // namespace text
