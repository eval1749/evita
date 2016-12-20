// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/base/line_number_cache.h"

#include "base/strings/utf_string_conversions.h"
#include "joana/base/memory/zone.h"
#include "joana/base/source_code_factory.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace joana {

class LineNumberCacheTest : public ::testing::Test {
 protected:
  LineNumberCacheTest();
  ~LineNumberCacheTest() override = default;

  const SourceCode& NewSourceCode(base::StringPiece source_text);

 private:
  Zone zone_;
  SourceCode::Factory factory_;

  DISALLOW_COPY_AND_ASSIGN(LineNumberCacheTest);
};

LineNumberCacheTest::LineNumberCacheTest()
    : zone_("LineNumberCacheTest"), factory_(&zone_) {}

const SourceCode& LineNumberCacheTest::NewSourceCode(
    base::StringPiece source_text) {
  const auto& source_text16 = base::UTF8ToUTF16(source_text);
  return factory_.New(base::FilePath(), base::StringPiece16(source_text16));
}

TEST_F(LineNumberCacheTest, Get) {
  const auto& source_code = NewSourceCode("01\n34\n6789\n");
  LineNumberCache cache(source_code);

  EXPECT_EQ(std::make_pair(1, 0), cache.Get(0));
  EXPECT_EQ(std::make_pair(1, 1), cache.Get(1)) << "no cache update";

  EXPECT_EQ(std::make_pair(3, 0), cache.Get(6)) << "populate cache";
  EXPECT_EQ(std::make_pair(3, 1), cache.Get(7)) << "scan";
  EXPECT_EQ(std::make_pair(3, 2), cache.Get(8)) << "scan";
  EXPECT_EQ(std::make_pair(3, 3), cache.Get(9)) << "scan";

  EXPECT_EQ(std::make_pair(1, 1), cache.Get(1)) << "cache hit";
  EXPECT_EQ(std::make_pair(1, 2), cache.Get(2)) << "cache hit";

  EXPECT_EQ(std::make_pair(2, 0), cache.Get(3)) << "cache hit";
  EXPECT_EQ(std::make_pair(2, 1), cache.Get(4)) << "cache hit";
  EXPECT_EQ(std::make_pair(2, 2), cache.Get(5)) << "cache hit";

  EXPECT_EQ(std::make_pair(3, 0), cache.Get(6)) << "cache hit";
  EXPECT_EQ(std::make_pair(3, 1), cache.Get(7)) << "cache hit";
  EXPECT_EQ(std::make_pair(3, 2), cache.Get(8)) << "cache hit";
  EXPECT_EQ(std::make_pair(3, 3), cache.Get(9)) << "cache hit";
}

}  // namespace joana
