// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma warning(push)
#pragma warning(disable: 4365 4625 4626 4826)
#include "gtest/gtest.h"
#pragma warning(pop)

#include "evita/spellchecker/hunspell_engine.h"

namespace {

class HunspellEngineTest : public ::testing::Test {
  protected: HunspellEngineTest() {
  }
  public: virtual ~HunspellEngineTest() {
  }

  protected: spellchecker::SpellingEngine* spelling_engine() const {
    return spellchecker::HunspellEngine::instance();
  }

  DISALLOW_COPY_AND_ASSIGN(HunspellEngineTest);
};

TEST_F(HunspellEngineTest, CheckSpelling) {
  EXPECT_TRUE(spelling_engine()->CheckSpelling(L"word"));
  EXPECT_FALSE(spelling_engine()->CheckSpelling(L"foobarbaz"));
}


TEST_F(HunspellEngineTest, GetSpellingSuggestions) {
  auto const suggestions = spelling_engine()->GetSpellingSuggestions(L"wrod");
  EXPECT_EQ(5u, suggestions.size());
  EXPECT_EQ(L"word", suggestions.size() >= 1u ? suggestions[0] : L"");
}

}  // namespace
