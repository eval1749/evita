// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "base/strings/string_piece.h"
#include "gtest/gtest.h"

namespace {

TEST(StringPiece, Constructors) {
  const char array_sample[] = "abcdefghi";
  const char* cstr_sample = "abcdefghi";
  std::string sample(cstr_sample);

  base::StringPiece piece0;
  EXPECT_EQ(piece0.data(), nullptr);
  EXPECT_EQ(piece0.size(), 0u);

  base::StringPiece piece1(cstr_sample);
  EXPECT_STREQ(piece1.data(), cstr_sample);
  EXPECT_EQ(piece1.size(), ::lstrlenA(cstr_sample));

  base::StringPiece piece2(array_sample);
  EXPECT_STREQ(piece2.data(), array_sample);
  EXPECT_EQ(piece2.size(), arraysize(array_sample) - 1);

  base::StringPiece piece3(cstr_sample + 3, 3);
  EXPECT_EQ(piece3.size(), 3);
  EXPECT_EQ(piece3[0], cstr_sample[3]);
  EXPECT_EQ(piece3[1], cstr_sample[4]);
  EXPECT_EQ(piece3[2], cstr_sample[5]);

  base::StringPiece piece4(sample);
  EXPECT_EQ(piece4.size(), sample.size());
  EXPECT_EQ(piece4[0], sample[0]);
  EXPECT_EQ(piece4[1], sample[1]);
  EXPECT_EQ(piece4[2], sample[2]);

  base::StringPiece piece5(sample.begin() + 1, sample.end() - 1);
  EXPECT_EQ(piece5.size(), sample.size() - 2);
  EXPECT_EQ(piece5[0], sample[1]);
  EXPECT_EQ(piece5[1], sample[2]);
  EXPECT_EQ(piece5[2], sample[3]);
}

TEST(StringPiece, Equal) {
  base::StringPiece piece1("piece1");
  base::StringPiece piece2("piece2");
  base::StringPiece piece3("piece1");
  EXPECT_EQ(piece1, piece1);
  EXPECT_NE(piece1, piece2);
  EXPECT_EQ(piece1, piece3);
}

TEST(StringPiece16, Constructors) {
  const base::char16 array_sample[] = L"abcdefghi";
  const base::char16* cstr_sample = L"abcdefghi";
  base::string16 sample(cstr_sample);

  base::StringPiece16 piece0;
  EXPECT_EQ(piece0.data(), nullptr);
  EXPECT_EQ(piece0.size(), 0u);

  base::StringPiece16 piece1(cstr_sample);
  EXPECT_STREQ(piece1.data(), cstr_sample);
  EXPECT_EQ(piece1.size(), ::lstrlenW(cstr_sample));

  base::StringPiece16 piece2(array_sample);
  EXPECT_STREQ(piece2.data(), array_sample);
  EXPECT_EQ(piece2.size(), arraysize(array_sample) - 1);

  base::StringPiece16 piece3(cstr_sample + 3, 3);
  EXPECT_EQ(piece3.size(), 3);
  EXPECT_EQ(piece3[0], cstr_sample[3]);
  EXPECT_EQ(piece3[1], cstr_sample[4]);
  EXPECT_EQ(piece3[2], cstr_sample[5]);

  base::StringPiece16 piece4(sample);
  EXPECT_EQ(piece4.size(), sample.size());
  EXPECT_EQ(piece4[0], sample[0]);
  EXPECT_EQ(piece4[1], sample[1]);
  EXPECT_EQ(piece4[2], sample[2]);

  base::StringPiece16 piece5(sample.begin() + 1, sample.end() - 1);
  EXPECT_EQ(piece5.size(), sample.size() - 2);
  EXPECT_EQ(piece5[0], sample[1]);
  EXPECT_EQ(piece5[1], sample[2]);
  EXPECT_EQ(piece5[2], sample[3]);
}


TEST(StringPiece16, Equal) {
  base::StringPiece16 piece1(L"piece1");
  base::StringPiece16 piece2(L"piece2");
  base::StringPiece16 piece3(L"piece1");
  EXPECT_TRUE(piece1 == piece1);
  EXPECT_FALSE(piece1 == piece2);
  EXPECT_TRUE(piece1 == piece3);
}

}  // namespace
