// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma warning(push)
#pragma warning(disable: 4365 4625 4626 4826)
#include "gtest/gtest.h"
#pragma warning(pop)

#include "evita/text/regex_matcher.h"
#include "evita/text/buffer.h"
#include "evita/text/modes/plain_text_mode.h"
#include "evita/text/range.h"
#include "evita/text/search_and_replace_model.h"

namespace {
class RegexMatchTest : public ::testing::Test {
  private: std::unique_ptr<text::Buffer> buffer_;

  protected: RegexMatchTest()
      : buffer_(new text::Buffer(L"*test*", new text::PlainTextMode())) {
  }
  public: virtual ~RegexMatchTest() {
  }

  protected: text::Buffer* buffer() const { return buffer_.get(); }

  private: void SetUp() override {
    auto const range = buffer()->CreateRange(0, 0);
                  //0123456789012345678
    range->SetText(L"foo bar baz foo bar");
  }

  DISALLOW_COPY_AND_ASSIGN(RegexMatchTest);
};

TEST_F(RegexMatchTest, FirstMatch_failed) {
  SearchParameters params;
  params.search_text_ = L"quux";
  params.m_rgf = 0;
  text::RegexMatcher matcher(&params, buffer(), 0, buffer()->GetEnd());
  EXPECT_FALSE(matcher.FirstMatch());
  EXPECT_EQ(nullptr, matcher.GetMatched(0));
}

TEST_F(RegexMatchTest, FirstMatch_succeeded) {
  SearchParameters params;
  params.search_text_ = L"baz";
  params.m_rgf = 0;
  text::RegexMatcher matcher(&params, buffer(), 0, buffer()->GetEnd());
  EXPECT_TRUE(matcher.FirstMatch());

  auto const matched_range = matcher.GetMatched(0);
  EXPECT_EQ(8, matched_range->GetStart());
  EXPECT_EQ(11, matched_range->GetEnd());
}

TEST_F(RegexMatchTest, NextMatch_failed) {
  SearchParameters params;
  params.search_text_ = L"baz";
  params.m_rgf = 0;
  text::RegexMatcher matcher(&params, buffer(), 0, buffer()->GetEnd());
  EXPECT_TRUE(matcher.FirstMatch());
  matcher.GetMatched(0)->Collapse(Collapse_End);
  EXPECT_FALSE(matcher.NextMatch());
}

TEST_F(RegexMatchTest, NextMatch_succeeded) {
  SearchParameters params;
  params.search_text_ = L"foo";
  params.m_rgf = 0;
  text::RegexMatcher matcher(&params, buffer(), 0, buffer()->GetEnd());
  EXPECT_TRUE(matcher.FirstMatch());
  matcher.GetMatched(0)->Collapse(Collapse_End);
  EXPECT_TRUE(matcher.NextMatch());

  auto const matched_range = matcher.GetMatched(0);
  EXPECT_EQ(12, matched_range->GetStart());
  EXPECT_EQ(15, matched_range->GetEnd());
}

TEST_F(RegexMatchTest, WrapMatch) {
  SearchParameters params;
  params.search_text_ = L"baz";
  params.m_rgf = SearchFlag_Whole;
  text::RegexMatcher matcher(&params, buffer(), 12, buffer()->GetEnd());
  EXPECT_FALSE(matcher.FirstMatch());
  EXPECT_TRUE(matcher.WrapMatch());
}

}  // namespace
