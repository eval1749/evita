// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/public/source_code_line.h"

#include "base/strings/utf_string_conversions.h"
#include "joana/public/memory/zone.h"
#include "joana/public/source_code_factory.h"
#include "joana/public/source_code_line_cache.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace joana {

class SourceCodeLineTest : public ::testing::Test {
 protected:
  SourceCodeLineTest();
  ~SourceCodeLineTest() override = default;

  const SourceCode& NewSourceCode(base::StringPiece source_text);
  SourceCodeLine NewSourceCodeLine(const SourceCode& source_code,
                                   int start,
                                   int end,
                                   int number);

 private:
  Zone zone_;
  SourceCode::Factory factory_;

  DISALLOW_COPY_AND_ASSIGN(SourceCodeLineTest);
};

SourceCodeLineTest::SourceCodeLineTest()
    : zone_("SourceCodeLineTest"), factory_(&zone_) {}

const SourceCode& SourceCodeLineTest::NewSourceCode(
    base::StringPiece source_text) {
  const auto& source_text16 = base::UTF8ToUTF16(source_text);
  return factory_.New(base::FilePath(), base::StringPiece16(source_text16));
}

SourceCodeLine SourceCodeLineTest::NewSourceCodeLine(
    const SourceCode& source_code,
    int start,
    int end,
    int number) {
  return SourceCodeLine(source_code.Slice(start, end), number);
}

TEST_F(SourceCodeLineTest, Get) {
  const auto& source_code = NewSourceCode("01\n34\n6789\n");
  SourceCodeLine::Cache cache(source_code);

  EXPECT_EQ(NewSourceCodeLine(source_code, 0, 3, 1), cache.Get(0));
  EXPECT_EQ(NewSourceCodeLine(source_code, 0, 3, 1), cache.Get(1))
      << "no cache update";

  EXPECT_EQ(NewSourceCodeLine(source_code, 6, 11, 3), cache.Get(6))
      << "populate cache";
  EXPECT_EQ(NewSourceCodeLine(source_code, 6, 11, 3), cache.Get(7)) << "scan";
  EXPECT_EQ(NewSourceCodeLine(source_code, 6, 11, 3), cache.Get(8)) << "scan";
  EXPECT_EQ(NewSourceCodeLine(source_code, 6, 11, 3), cache.Get(9)) << "scan";

  EXPECT_EQ(NewSourceCodeLine(source_code, 0, 3, 1), cache.Get(1))
      << "cache hit";
  EXPECT_EQ(NewSourceCodeLine(source_code, 0, 3, 1), cache.Get(2))
      << "cache hit";

  EXPECT_EQ(NewSourceCodeLine(source_code, 3, 6, 2), cache.Get(3))
      << "cache hit";
  EXPECT_EQ(NewSourceCodeLine(source_code, 3, 6, 2), cache.Get(4))
      << "cache hit";
  EXPECT_EQ(NewSourceCodeLine(source_code, 3, 6, 2), cache.Get(5))
      << "cache hit";

  EXPECT_EQ(NewSourceCodeLine(source_code, 6, 11, 3), cache.Get(6))
      << "cache hit";
  EXPECT_EQ(NewSourceCodeLine(source_code, 6, 11, 3), cache.Get(7))
      << "cache hit";
  EXPECT_EQ(NewSourceCodeLine(source_code, 6, 11, 3), cache.Get(8))
      << "cache hit";
  EXPECT_EQ(NewSourceCodeLine(source_code, 6, 11, 3), cache.Get(9))
      << "cache hit";
}

TEST_F(SourceCodeLineTest, GetOnEmpty) {
  const auto& source_code = NewSourceCode("");
  SourceCodeLine::Cache cache(source_code);

  EXPECT_EQ(NewSourceCodeLine(source_code, 0, 0, 1), cache.Get(0));
}

TEST_F(SourceCodeLineTest, GetOnNoNewline) {
  const auto& source_code = NewSourceCode("ab");
  SourceCodeLine::Cache cache(source_code);

  EXPECT_EQ(NewSourceCodeLine(source_code, 0, 2, 1), cache.Get(0));
  EXPECT_EQ(NewSourceCodeLine(source_code, 0, 2, 1), cache.Get(1));
  EXPECT_EQ(NewSourceCodeLine(source_code, 0, 2, 1), cache.Get(2));
}

}  // namespace joana
