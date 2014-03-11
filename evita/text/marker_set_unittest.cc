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
  private: MarketSet marker_set_;

  protected: MarkerSetTest() {
  }
  public: virtual ~MarkerSetTest() {
  }

  protected: MarkerSet* marker_set() { return &marker_set_; }

  DISALLOW_COPY_AND_ASSIGN(MarkerSetTest);
};

TEST_F(MarkerSetTest, SetMarker) {
}

}  // namespace
