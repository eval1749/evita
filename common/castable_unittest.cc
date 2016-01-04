// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common/castable.h"
#include "gtest/gtest.h"

namespace {

class Class1 : public common::Castable<Class1> {
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
  EXPECT_TRUE(instance1.is<Class1>());
  EXPECT_FALSE(instance1.is<Class2>());
  EXPECT_TRUE(instance2.is<Class1>());
  EXPECT_TRUE(instance3.is<Class1>());
  EXPECT_TRUE(instance4.is<Class1>());
  EXPECT_TRUE(instance3.is<Class2>());
  EXPECT_TRUE(instance4.is<Class2>());
  EXPECT_FALSE(instance4.is<Class3>());
}

}  // namespace
