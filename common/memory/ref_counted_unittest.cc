// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "common/memory/ref_counted.h"
#include "gtest/gtest.h"

namespace {

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

TEST(RefCounted, Basic) {
  auto destructed = false;
  auto const sample = new MyRefCounted(&destructed);
  sample->AddRef();
  sample->Release();
  EXPECT_TRUE(destructed);
}

}  // namespace
