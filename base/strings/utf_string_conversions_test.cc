// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "base/strings/utf_string_conversions.h"

#include "base/strings/string_util.h"
#include "gtest/gtest.h"

#include <iostream>

namespace {

TEST(ASCIIToUTF16, Basic) {
  base::string16 expected_result(L"abc");
  auto result = base::ASCIIToUTF16("abc");
  EXPECT_EQ(result, expected_result);
}

}  // namespace
