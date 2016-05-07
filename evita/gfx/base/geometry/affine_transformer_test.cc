// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/gfx/base/geometry/affine_transformer.h"
#include "testing/gtest/include/gtest/gtest.h"

#include "evita/gfx/base/geometry/float_rect.h"

namespace gfx {

TEST(AffineTransformerTest, MapRect) {
  AffineTransformer transformer;
  transformer.Translate(30, 20);
  EXPECT_EQ(FloatRect(FloatPoint(30, 20), FloatSize(40, 50)),
            transformer.MapRect(FloatRect(FloatSize(40, 50))));
}

}  // namespace gfx
