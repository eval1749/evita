// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "common/memory/ref_counted.h"
#include "gtest/gtest.h"

namespace {

class MyRefCounted : public common::RefCounted<MyRefCounted> {
  private: bool* destructed_;

  public: MyRefCounted(bool* destructed) : destructed_(destructed) {
    *destructed_ = false;
  }

  public: ~MyRefCounted() {
    *destructed_ = true;
  }
};

TEST(RefCounted, Basic) {
  bool destructed = false;
  auto const sample = new MyRefCounted(&destructed);
  sample->AddRef();
  sample->Release();
  EXPECT_TRUE(destructed);
}

} // namespace
