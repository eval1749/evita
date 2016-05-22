// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <vector>

#include "evita/text/layout/text_layout_test_base.h"

#include "base/strings/string16.h"
#include "evita/base/strings/atomic_string.h"
#include "evita/text/layout/line/root_inline_box.h"
#include "evita/text/layout/line/root_inline_box_cache.h"
#include "evita/text/layout/text_format_context.h"
#include "evita/text/layout/text_formatter.h"
#include "evita/text/models/buffer.h"
#include "evita/text/models/marker_set.h"
#include "evita/text/models/static_range.h"

namespace layout {

//////////////////////////////////////////////////////////////////////
//
// RootInlineBoxCacheTest
//
class RootInlineBoxCacheTest : public TextLayoutTestBase {
 protected:
  RootInlineBoxCacheTest();
  ~RootInlineBoxCacheTest() override = default;

  RootInlineBoxCache* cache() const { return cache_.get(); }
  const std::vector<RootInlineBox*> lines() const { return lines_; }

  void PopulateCache(const base::string16& text);
  void SetBounds(const gfx::RectF& new_bounds);

 private:
  // |cache_| takes |buffer_| and |markers_|.
  const std::unique_ptr<RootInlineBoxCache> cache_;
  std::vector<RootInlineBox*> lines_;

  DISALLOW_COPY_AND_ASSIGN(RootInlineBoxCacheTest);
};

RootInlineBoxCacheTest::RootInlineBoxCacheTest()
    : cache_(new RootInlineBoxCache(*buffer(), *markers())) {
  set_bounds(gfx::RectF(gfx::SizeF(640.0f, 480.0f)));
}

void RootInlineBoxCacheTest::PopulateCache(const base::string16& text) {
  buffer()->InsertBefore(text::Offset(), text);
  cache_->Invalidate(bounds(), zoom());
  TextFormatter formatter(FormatContextFor(text::Offset(0)));
  for (;;) {
    const auto line = cache_->Register(formatter.FormatLine());
    lines_.push_back(line);
    if (line->IsEndOfDocument())
      break;
  }
}

void RootInlineBoxCacheTest::SetBounds(const gfx::RectF& new_bounds) {
  set_bounds(new_bounds);
  cache_->Invalidate(new_bounds, zoom());
}

TEST_F(RootInlineBoxCacheTest, DidChangeStyle) {
  PopulateCache(L"foo\nbar\nbaz");
  xcss::Style style(xcss::Color(255, 0, 0), xcss::Color(255, 255, 255));
  buffer()->syntax_markers()->InsertMarker(
      text::StaticRange(*buffer(), text::Offset(1), text::Offset(2)),
      base::AtomicString(L"keyword"));
  EXPECT_EQ(nullptr, cache()->FindLine(text::Offset(0)));
  EXPECT_EQ(nullptr, cache()->FindLine(text::Offset(1)));
  EXPECT_EQ(lines()[1], cache()->FindLine(text::Offset(4)))
      << "second line is still in cache.";
  EXPECT_EQ(lines()[1], cache()->FindLine(text::Offset(5)));
  EXPECT_EQ(lines()[2], cache()->FindLine(text::Offset(8)));
}

TEST_F(RootInlineBoxCacheTest, DidDeleteAt) {
  PopulateCache(L"foo\nbar\nbaz");
  buffer()->Delete(text::Offset(1), text::Offset(3));
  //       01_2345_678
  // text: f\nbar\nbaz
  EXPECT_EQ(nullptr, cache()->FindLine(text::Offset(0)));
  EXPECT_EQ(nullptr, cache()->FindLine(text::Offset(1)));
  EXPECT_EQ(lines()[1], cache()->FindLine(text::Offset(2)))
      << "second line is still in cache.";
  EXPECT_EQ(lines()[1], cache()->FindLine(text::Offset(3)));
  EXPECT_EQ(lines()[2], cache()->FindLine(text::Offset(6)));
}

TEST_F(RootInlineBoxCacheTest, DidDeleteAtToJoiningLines) {
  PopulateCache(L"foo\nbar\nbaz");
  buffer()->Delete(text::Offset(3), text::Offset(4));
  //       0123456_78
  // text: foobar\nbaz
  EXPECT_EQ(nullptr, cache()->FindLine(text::Offset(0)));
  EXPECT_EQ(nullptr, cache()->FindLine(text::Offset(1)));
  EXPECT_EQ(nullptr, cache()->FindLine(text::Offset(3)));
  EXPECT_EQ(lines()[2], cache()->FindLine(text::Offset(7)))
      << "line 'baz' is still in cache.";
}

TEST_F(RootInlineBoxCacheTest, DidDeleteAtWithLineWrap) {
  SetBounds(gfx::RectF(gfx::SizeF(60.0f, 100.0f)));
  PopulateCache(L"0123456789\nabcd");
  // View:
  //    01234
  //    56789\n
  //    abcd
  buffer()->Delete(text::Offset(2), text::Offset(5));
  //       01234567_8901
  // text: 0156789\nabcd
  EXPECT_EQ(nullptr, cache()->FindLine(text::Offset(0)));
  EXPECT_EQ(nullptr, cache()->FindLine(text::Offset(1)));
  EXPECT_EQ(nullptr, cache()->FindLine(text::Offset(7)));
  EXPECT_EQ(lines()[2], cache()->FindLine(text::Offset(8)))
      << "second line is still in cache.";
  EXPECT_EQ(lines()[2], cache()->FindLine(text::Offset(9)));
}

TEST_F(RootInlineBoxCacheTest, DidInsertBefore) {
  PopulateCache(L"foo\nbar\nbaz");
  buffer()->InsertBefore(text::Offset(2), L"AB");
  //       01234_56789_012
  // text: foABo\nbar\nbaz
  EXPECT_EQ(nullptr, cache()->FindLine(text::Offset(0)));
  EXPECT_EQ(nullptr, cache()->FindLine(text::Offset(1)));
  EXPECT_EQ(lines()[1], cache()->FindLine(text::Offset(6)))
      << "second line is still in cache.";
  EXPECT_EQ(lines()[1], cache()->FindLine(text::Offset(7)));
  EXPECT_EQ(lines()[2], cache()->FindLine(text::Offset(10)));
  EXPECT_EQ(lines()[2], cache()->FindLine(text::Offset(11)));
}

TEST_F(RootInlineBoxCacheTest, DidInsertBeforeWithLineWrap) {
  SetBounds(gfx::RectF(gfx::SizeF(60.0f, 100.0f)));
  PopulateCache(L"0123456789\nabcd");
  // View:
  //    01234
  //    56789\n
  //    abcd
  buffer()->InsertBefore(text::Offset(2), L"AB");
  //       01234567890012_3456
  // text: 01AB234566789\nabcd
  EXPECT_EQ(nullptr, cache()->FindLine(text::Offset(0)));
  EXPECT_EQ(nullptr, cache()->FindLine(text::Offset(1)));
  EXPECT_EQ(nullptr, cache()->FindLine(text::Offset(11)));
  EXPECT_EQ(nullptr, cache()->FindLine(text::Offset(12)));
  EXPECT_EQ(lines()[2], cache()->FindLine(text::Offset(13)))
      << "second line is still in cache.";
  EXPECT_EQ(lines()[2], cache()->FindLine(text::Offset(14)));
}

TEST_F(RootInlineBoxCacheTest, FindLine) {
  PopulateCache(L"foo\nbar\nbaz");
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

TEST_F(RootInlineBoxCacheTest, Invalidate) {
  PopulateCache(L"0\n123456");
  cache()->Invalidate(gfx::RectF(gfx::SizeF(1000.0f, 1000.0f)), zoom());
  EXPECT_EQ(lines()[0], cache()->FindLine(text::Offset(0)))
      << "Resize to large doesn't clear cache.";
  EXPECT_EQ(lines()[1], cache()->FindLine(text::Offset(2)));

  cache()->Invalidate(gfx::RectF(gfx::SizeF(40.0f, 200.0f)), zoom());
  EXPECT_EQ(lines()[0], cache()->FindLine(text::Offset(0)))
      << "Still fit in bounds";
  EXPECT_EQ(nullptr, cache()->FindLine(text::Offset(2)));
}

TEST_F(RootInlineBoxCacheTest, InvalidateWithLineWrap) {
  SetBounds(gfx::RectF(gfx::SizeF(40.0f, 100.0f)));
  PopulateCache(L"0\n123456");
  SetBounds(gfx::RectF(gfx::SizeF(200.0f, 100.0f)));

  EXPECT_EQ(lines()[0], cache()->FindLine(text::Offset(0)))
      << "Resize to large doesn't clear cache.";
  EXPECT_EQ(nullptr, cache()->FindLine(text::Offset(2)))
      << "Wrapped line should not be in cache.";
}

}  // namespace layout
