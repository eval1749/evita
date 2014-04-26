// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common/strings/atomic_string.h"
#include "gtest/gtest.h"

namespace {

TEST(AtomicStringTest, ctor) {
  common::AtomicString sample1(L"sample1");
  common::AtomicString sample1_another(L"sample1");
  common::AtomicString sample2(L"sample2");
  EXPECT_EQ(sample1, sample1_another);
  EXPECT_NE(sample1, sample2);
}

}  // namespace
