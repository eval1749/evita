// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma warning(push)
#pragma warning(disable: 4365 4625 4626 4826)
#include "gtest/gtest.h"
#pragma warning(pop)

#include "evita/text/marker_set.h"

namespace {

class MarkerSetTest : public ::testing::Test {
  protected: enum Spelling {
    None,
    Correct,
    Misspelled,
  };

  private: text::MarkerSet marker_set_;

  protected: MarkerSetTest() {
  }
  public: virtual ~MarkerSetTest() {
  }

  protected: text::MarkerSet* marker_set() { return &marker_set_; }

  protected: text::Marker GetAt(Posn offset) {
    return marker_set_.GetMarkerAt(offset);
  }

  protected: void Insert(text::Posn start, text::Posn end, int type) {
    marker_set()->InsertMarker(start, end, type);
  }

  DISALLOW_COPY_AND_ASSIGN(MarkerSetTest);
};

TEST_F(MarkerSetTest, GetMarkerAt) {
  Insert(5, 10, Correct);
  EXPECT_EQ(None, marker_set()->GetMarkerAt(0).type());
  EXPECT_EQ(None, marker_set()->GetMarkerAt(4).type());
  EXPECT_EQ(Correct, marker_set()->GetMarkerAt(5).type());
  EXPECT_EQ(Correct, marker_set()->GetMarkerAt(9).type());
  EXPECT_EQ(None, marker_set()->GetMarkerAt(10).type());
  EXPECT_EQ(None, marker_set()->GetMarkerAt(11).type());
}

TEST_F(MarkerSetTest, InsertMarker_cover) {
  // before: --CC--
  // insert: -MMMM--
  // after:  -MMMM--
  Insert(150, 250, Correct);
  Insert(100, 300, Misspelled);
  EXPECT_EQ(text::Marker(Misspelled, 100, 300), GetAt(150));
  EXPECT_EQ(text::Marker(Misspelled, 100, 300), GetAt(250));
}

TEST_F(MarkerSetTest, InsertMarker_cross_left) {
  // before: ---MM--
  // insert: --CC---
  // after:  --CCM--
  Insert(200, 300, Correct);
  Insert(150, 250, Misspelled);
  EXPECT_EQ(text::Marker(Misspelled, 150, 250), GetAt(150));
  EXPECT_EQ(text::Marker(Correct, 250, 300), GetAt(250));
}

TEST_F(MarkerSetTest, InsertMarker_cross_right) {
  // before: --MMMM---
  // insert: ----CCC--
  // after:  --MMCCC--
  Insert(100, 250, Misspelled);
  Insert(200, 300, Correct);
  EXPECT_EQ(text::Marker(Misspelled, 100, 200), GetAt(150));
  EXPECT_EQ(text::Marker(Correct, 200, 300), GetAt(200));
  EXPECT_EQ(text::Marker(Misspelled, 200, 300), GetAt(250));
}

TEST_F(MarkerSetTest, InsertMarker_disjoint) {
  // before: --CC--
  // insert: ------CC--
  // after:  --CC--CC--
  Insert(100, 200, Correct);
  Insert(300, 400, Correct);
  EXPECT_EQ(text::Marker(Correct, 100, 200), GetAt(150));
  EXPECT_EQ(text::Marker(Correct, 100, 200), GetAt(350));
}

TEST_F(MarkerSetTest, InsertMarker_split) {
  // before: -MMMM--
  // insert: --CC--
  // after:  -MCCM--
  Insert(100, 300, Misspelled);
  Insert(150, 250, Correct);
  EXPECT_EQ(text::Marker(Misspelled, 100, 150), GetAt(100));
  EXPECT_EQ(text::Marker(Misspelled, 100, 150), GetAt(125));
  EXPECT_EQ(text::Marker(Correct, 150, 250), GetAt(150));
  EXPECT_EQ(text::Marker(Correct, 150, 250), GetAt(200));
  EXPECT_EQ(text::Marker(Misspelled, 250, 300), GetAt(250));
}

}  // namespace
