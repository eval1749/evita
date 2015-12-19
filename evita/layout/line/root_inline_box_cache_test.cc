// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <vector>

#include "base/strings/string16.h"
#include "evita/dom/lock.h"
#include "evita/editor/dom_lock.h"
#include "evita/layout/line/root_inline_box.h"
#include "evita/layout/line/root_inline_box_cache.h"
#include "evita/layout/text_formatter.h"
#include "evita/text/buffer.h"
#include "gtest/gtest.h"

namespace layout {

//////////////////////////////////////////////////////////////////////
//
// RootInlineBoxCacheTest
//
class RootInlineBoxCacheTest : public ::testing::Test {
 protected:
  RootInlineBoxCacheTest();
  ~RootInlineBoxCacheTest() override = default;

  const gfx::RectF& bounds() const { return bounds_; }
  text::Buffer* buffer() const { return buffer_.get(); }
  RootInlineBoxCache* cache() const { return cache_.get(); }
  const std::vector<RootInlineBox*> lines() const { return lines_; }
  float zoom() const { return zoom_; }

  void PopulateCache(const base::string16& text);

 private:
  gfx::RectF bounds_;
  const std::unique_ptr<text::Buffer> buffer_;
  const std::unique_ptr<RootInlineBoxCache> cache_;
  std::vector<RootInlineBox*> lines_;
  float zoom_ = 1.0f;

  DISALLOW_COPY_AND_ASSIGN(RootInlineBoxCacheTest);
};

RootInlineBoxCacheTest::RootInlineBoxCacheTest()
    : bounds_(gfx::PointF(100.0f, 200.0f), gfx::SizeF(640.0f, 480.0f)),
      buffer_(new text::Buffer()),
      cache_(new RootInlineBoxCache(buffer())) {}

void RootInlineBoxCacheTest::PopulateCache(const base::string16& text) {
  {
    DOM_AUTO_LOCK_SCOPE();
    buffer()->InsertBefore(text::Offset(), text);
  }
  UI_DOM_AUTO_LOCK_SCOPE();
  cache_->Invalidate(bounds_, zoom_);
  TextFormatter formatter(buffer(), text::Offset(0), bounds_, zoom_);
  for (;;) {
    const auto& line = formatter.FormatLine();
    cache_->Register(line);
    lines_.push_back(line.get());
    if (line->IsEndOfDocument())
      break;
  }
}

TEST_F(RootInlineBoxCacheTest, FindLine) {
  PopulateCache(L"foo\nbar\nbaz");
  UI_DOM_AUTO_LOCK_SCOPE();
  EXPECT_FALSE(cache()->IsDirty(bounds(), zoom()));
  EXPECT_EQ(lines()[0], cache()->FindLine(text::Offset(0)));
  EXPECT_EQ(lines()[0], cache()->FindLine(text::Offset(1)));
  EXPECT_EQ(lines()[0], cache()->FindLine(text::Offset(2)));
  EXPECT_EQ(lines()[0], cache()->FindLine(text::Offset(3)));
  EXPECT_EQ(lines()[1], cache()->FindLine(text::Offset(4)));
  EXPECT_EQ(lines()[1], cache()->FindLine(text::Offset(5)));
  EXPECT_EQ(lines()[1], cache()->FindLine(text::Offset(6)));
  EXPECT_EQ(lines()[1], cache()->FindLine(text::Offset(7)));
  EXPECT_EQ(lines()[2], cache()->FindLine(text::Offset(8)));
  EXPECT_EQ(lines()[2], cache()->FindLine(text::Offset(9)));
  EXPECT_EQ(lines()[2], cache()->FindLine(text::Offset(10)));
  EXPECT_EQ(lines()[2], cache()->FindLine(text::Offset(11))) << "buffer.length";
  EXPECT_FALSE(cache()->FindLine(text::Offset(12))) << "buffer.length + 1";
  EXPECT_FALSE(cache()->FindLine(text::Offset(99)))
      << "grater than buffer.length";
}

}  // namespace layout
