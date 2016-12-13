// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "joana/public/source_code_range.h"

#include "base/strings/utf_string_conversions.h"
#include "joana/public/memory/zone.h"
#include "joana/public/source_code_factory.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace joana {

class SourceCodeRangeTest : public ::testing::Test {
 protected:
  SourceCodeRangeTest();
  ~SourceCodeRangeTest() override = default;

  const SourceCode& NewSourceCode(base::StringPiece source_text);

 private:
  Zone zone_;
  SourceCode::Factory factory_;

  DISALLOW_COPY_AND_ASSIGN(SourceCodeRangeTest);
};

SourceCodeRangeTest::SourceCodeRangeTest()
    : zone_("SourceCodeRangeTest"), factory_(&zone_) {}

const SourceCode& SourceCodeRangeTest::NewSourceCode(
    base::StringPiece source_text) {
  const auto& source_text16 = base::UTF8ToUTF16(source_text);
  return factory_.New(base::FilePath(), base::StringPiece16(source_text16));
}

TEST_F(SourceCodeRangeTest, Merge) {
  const auto& source_code = NewSourceCode("0123456789abcdef");
  const auto& range1 = source_code.Slice(1, 3);
  const auto& range2 = source_code.Slice(5, 9);
  const auto& range3 = source_code.Slice(11, 14);
  EXPECT_EQ(source_code.Slice(1, 14),
            SourceCodeRange::Merge(range1, range2, range3));
  EXPECT_EQ(source_code.Slice(1, 14),
            SourceCodeRange::Merge(range2, range1, range3, range1));
}

}  // namespace joana
