// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/base/castable.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace joana {

class Class1 : public Castable<Class1> {
  DECLARE_CASTABLE_CLASS(Class1, Castable);
};

class Class2 : public Class1 {
  DECLARE_CASTABLE_CLASS(Class2, Class1);
};

class Class3 : public Class2 {
  DECLARE_CASTABLE_CLASS(Class3, Class2);
};

class Class4 : public Class2 {
  DECLARE_CASTABLE_CLASS(Class4, Class2);
};

TEST(Castable, All) {
  Class1 instance1;
  Class2 instance2;
  Class3 instance3;
  Class4 instance4;
  EXPECT_EQ(&instance4, instance4.TryAs<Class1>());
  EXPECT_TRUE(instance1.Is<Class1>());
  EXPECT_FALSE(instance1.Is<Class2>());
  EXPECT_TRUE(instance2.Is<Class1>());
  EXPECT_TRUE(instance3.Is<Class1>());
  EXPECT_TRUE(instance4.Is<Class1>());
  EXPECT_TRUE(instance3.Is<Class2>());
  EXPECT_TRUE(instance4.Is<Class2>());
  EXPECT_FALSE(instance4.Is<Class3>());
}

}  // namespace joana
