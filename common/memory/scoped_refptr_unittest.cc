// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "base/logging.h"
#include "common/memory/ref_counted.h"
#include "common/memory/scoped_refptr.h"
#include "gtest/gtest.h"

namespace {

using common::scoped_refptr;

class MyRefCounted final : public common::RefCounted<MyRefCounted> {
 public:
  explicit MyRefCounted(bool* destructed) : destructed_(destructed) {
    *destructed_ = false;
  }
  ~MyRefCounted() { *destructed_ = true; }

 private:
  bool* destructed_;

  DISALLOW_COPY_AND_ASSIGN(MyRefCounted);
};

class ScopedRefPtrTest : public ::testing::Test {
 public:
  ~ScopedRefPtrTest() {
    while (!destructed1_) {
      pointer1_->Release();
    }
    while (!destructed2_) {
      pointer2_->Release();
    }
  }

 protected:
  ScopedRefPtrTest()
      : pointer1_(new MyRefCounted(&destructed1_)),
        pointer2_(new MyRefCounted(&destructed2_)),
        destructed1_(false),
        destructed2_(false) {
    pointer2_->AddRef();
  }

  bool destructed1() const { return destructed1_; }
  MyRefCounted* pointer1() const { return pointer1_; }
  MyRefCounted* pointer2() const {
    pointer2_->AddRef();
    return pointer2_;
  }

 private:
  MyRefCounted* pointer1_;
  MyRefCounted* pointer2_;
  bool destructed1_;
  bool destructed2_;
};

TEST_F(ScopedRefPtrTest, Basic) {
  scoped_refptr<MyRefCounted> sample1;
  EXPECT_EQ(nullptr, sample1.get());

  {
    auto sample2 = common::make_scoped_refptr(pointer1());
    auto sample3 = common::make_scoped_refptr(pointer1());
    EXPECT_EQ(pointer1(), sample2.get());
    EXPECT_EQ(pointer1(), sample3.get());
  }
  EXPECT_TRUE(destructed1());
}

TEST_F(ScopedRefPtrTest, ConstructorConstRef) {
  {
    auto sample1 = common::make_scoped_refptr(pointer1());
    auto sample2(sample1);
    EXPECT_EQ(pointer1(), sample1.get());
    EXPECT_EQ(pointer1(), sample2.get());
  }
  EXPECT_TRUE(destructed1());
}

TEST_F(ScopedRefPtrTest, ConstructorPtr) {
  {
    scoped_refptr<MyRefCounted> sample1(pointer1());
    EXPECT_EQ(pointer1(), sample1.get());
  }
  EXPECT_TRUE(destructed1());
}

TEST_F(ScopedRefPtrTest, ConstructorRValue) {
  {
    scoped_refptr<MyRefCounted> sample1(pointer1());
    scoped_refptr<MyRefCounted> sample2(std::move(sample1));

    EXPECT_EQ(nullptr, sample1.get());
    EXPECT_EQ(pointer1(), sample2.get());
  }
  EXPECT_TRUE(destructed1());
}

TEST_F(ScopedRefPtrTest, AssignConstRef) {
  {
    auto sample1 = common::make_scoped_refptr(pointer1());
    scoped_refptr<MyRefCounted> sample2;
    sample2 = sample1;
    EXPECT_EQ(pointer1(), sample1.get());
    EXPECT_EQ(pointer1(), sample2.get());
  }
  EXPECT_TRUE(destructed1());
}

TEST_F(ScopedRefPtrTest, AssignPtr) {
  {
    scoped_refptr<MyRefCounted> sample1;
    sample1 = pointer1();
    EXPECT_EQ(pointer1(), sample1.get());
  }
  EXPECT_TRUE(destructed1());
}

TEST_F(ScopedRefPtrTest, AssignRValue) {
  {
    scoped_refptr<MyRefCounted> sample1(pointer1());
    scoped_refptr<MyRefCounted> sample2;
    sample2 = std::move(sample1);

    EXPECT_EQ(nullptr, sample1.get());
    EXPECT_EQ(pointer1(), sample2.get());
  }
  EXPECT_TRUE(destructed1());
}

TEST_F(ScopedRefPtrTest, OperatorEqualPtr) {
  auto sample1 = common::make_scoped_refptr(pointer1());
  EXPECT_TRUE(sample1 == pointer1());
  EXPECT_FALSE(sample1 == pointer2());
  EXPECT_FALSE(sample1 != pointer1());
  EXPECT_TRUE(sample1 != pointer2());
}

TEST_F(ScopedRefPtrTest, OperatorEqualRef) {
  auto sample1 = common::make_scoped_refptr(pointer1());
  auto sample2 = common::make_scoped_refptr(pointer2());
  auto sample3 = common::make_scoped_refptr(pointer1());
  EXPECT_TRUE(sample1 == sample1);
  EXPECT_FALSE(sample1 == sample2);
  EXPECT_TRUE(sample1 == sample3);
  EXPECT_FALSE(sample1 != sample1);
  EXPECT_TRUE(sample1 != sample2);
  EXPECT_FALSE(sample1 != sample3);
}

TEST_F(ScopedRefPtrTest, Release) {
  auto sample1 = common::make_scoped_refptr(pointer1());
  EXPECT_EQ(pointer1(), sample1.release());
  EXPECT_FALSE(destructed1());
}

TEST_F(ScopedRefPtrTest, Swap) {
  auto sample1 = common::make_scoped_refptr(pointer1());
  auto sample2 = common::make_scoped_refptr(pointer2());
  sample1.swap(sample2);
  EXPECT_EQ(pointer2(), sample1.get());
  EXPECT_EQ(pointer1(), sample2.get());
  EXPECT_FALSE(destructed1());

  auto pointer3 = pointer1();
  pointer3->AddRef();
  sample1.swap(&pointer3);
  EXPECT_EQ(pointer1(), sample1.get());
  EXPECT_EQ(pointer2(), pointer3);
  EXPECT_FALSE(destructed1());
}

}  // namespace
