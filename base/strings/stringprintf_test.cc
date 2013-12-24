// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "base/strings/stringprintf.h"
#include "gtest/gtest.h"

namespace {

TEST(StringPrintf, Basic) {
  EXPECT_EQ(base::StringPrintf("foo"), "foo");
  EXPECT_EQ(base::StringPrintf("wait %dms", 123), "wait 123ms");
}

TEST(StringPrintf, Basic16) {
  EXPECT_EQ(base::StringPrintf(L"foo"), L"foo");
  EXPECT_EQ(base::StringPrintf(L"wait %dms", 123), L"wait 123ms");
}

}  // namespace
