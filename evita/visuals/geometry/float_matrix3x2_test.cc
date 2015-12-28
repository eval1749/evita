// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/geometry/float_matrix3x2.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(FloatMatrix3x2Test, Equals) {
  FloatMatrix3x2 matrix0;
  FloatMatrix3x2 matrix1({1, 2, 3, 4, 5, 6});
  FloatMatrix3x2 matrix2({1, 2, 3, 4, 5, 6});
  FloatMatrix3x2 matrix3({6, 5, 4, 3, 2, 1});

  EXPECT_TRUE(matrix0 == matrix0);
  EXPECT_FALSE(matrix0 == matrix1);
  EXPECT_FALSE(matrix0 == matrix2);
  EXPECT_FALSE(matrix0 == matrix3);

  EXPECT_FALSE(matrix0 != matrix0);
  EXPECT_TRUE(matrix0 != matrix1);
  EXPECT_TRUE(matrix0 != matrix2);
  EXPECT_TRUE(matrix0 != matrix3);

  EXPECT_FALSE(matrix1 == matrix0);
  EXPECT_TRUE(matrix1 == matrix1);
  EXPECT_TRUE(matrix1 == matrix2);
  EXPECT_FALSE(matrix1 == matrix3);

  EXPECT_TRUE(matrix1 != matrix0);
  EXPECT_FALSE(matrix1 != matrix1);
  EXPECT_FALSE(matrix1 != matrix2);
  EXPECT_TRUE(matrix1 != matrix3);

  EXPECT_FALSE(matrix2 == matrix0);
  EXPECT_TRUE(matrix2 == matrix1);
  EXPECT_TRUE(matrix2 == matrix2);
  EXPECT_FALSE(matrix2 == matrix3);

  EXPECT_TRUE(matrix2 != matrix0);
  EXPECT_FALSE(matrix2 != matrix1);
  EXPECT_FALSE(matrix2 != matrix2);
  EXPECT_TRUE(matrix2 != matrix3);
}

TEST(FloatMatrix3x2Test, Inverse) {
  EXPECT_EQ(FloatMatrix3x2(), FloatMatrix3x2().Inverse());
  EXPECT_EQ(FloatMatrix3x2({1, 0, 0, 1, -10, -20}),
            FloatMatrix3x2({1, 0, 0, 1, 10, 20}).Inverse());
  EXPECT_EQ(FloatMatrix3x2({0.5, 0, 0, 0.5, 0, 0}),
            FloatMatrix3x2({2, 0, 0, 2, 0, 0}).Inverse());
  EXPECT_EQ(FloatMatrix3x2({0.5, 0, 0, 0.5, -1.5, -2}),
            FloatMatrix3x2({2, 0, 0, 2, 3, 4}).Inverse());
}

}  // namespace visuals
