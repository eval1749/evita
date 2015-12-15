// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_LAYOUT_LAYOUT_BLOCK_FLOW_H_
#define EVITA_LAYOUT_LAYOUT_BLOCK_FLOW_H_

#include <list>
#include <memory>

#include "evita/gfx_base.h"
#include "evita/layout/render_style.h"
#include "evita/text/offset.h"

namespace text {
class Buffer;
}

namespace layout {

class TextFormatter;
class RootInlineBox;
class RootInlineBoxCache;

//////////////////////////////////////////////////////////////////////
//
// LayoutBlockFlow
//
class LayoutBlockFlow final {
 public:
  explicit LayoutBlockFlow(text::Buffer* buffer);
  ~LayoutBlockFlow();

  const gfx::RectF& bounds() const { return bounds_; }
  bool dirty() const { return dirty_; }
  int format_counter() const { return format_counter_; }
  const std::list<RootInlineBox*>& lines() const { return lines_; }

  void DidChangeStyle(text::Offset offset, text::OffsetDelta length);
  void DidDeleteAt(text::Offset offset, text::OffsetDelta length);
  void DidInsertBefore(text::Offset offset, text::OffsetDelta length);
  // Returns end of line offset containing |text_offset|.
  text::Offset EndOfLine(text::Offset text_offset);
  void Format(text::Offset text_offset);
  // Returns true if text format is taken place.
  bool FormatIfNeeded();
  text::Offset GetEnd();
  text::Offset GetStart();
  text::Offset GetVisibleEnd();
  text::Offset HitTestPoint(gfx::PointF point);
  gfx::RectF HitTestTextPosition(text::Offset text_offset) const;
  bool IsPositionFullyVisible(text::Offset text_offset);
  text::Offset MapPointXToOffset(text::Offset text_offset, float point_x);
  // Returns true if this |LayoutBlockFlow| is modified.
  bool ScrollDown();
  // Returns true if this |LayoutBlockFlow| is modified.
  bool ScrollToPosition(text::Offset offset);
  // Returns true if this |LayoutBlockFlow| is modified.
  bool ScrollUp();
  void SetBounds(const gfx::RectF& new_bounds);
  void SetZoom(float new_zoom);
  bool ShouldFormat() const;
  // Returns start of line offset containing |text_offset|.
  text::Offset StartOfLine(text::Offset text_offset);

 private:
  void Append(RootInlineBox* line);
  // Returns true if discarded the first line.
  bool DiscardFirstLine();
  // Returns true if discarded the last line.
  bool DiscardLastLine();
  void EnsureLinePoints();
  RootInlineBox* FormatLine(TextFormatter* formatter);
  void InvalidateCache();
  void InvalidateLines(text::Offset offset);
  bool IsShowEndOfDocument() const;

  // Returns true if we need to format all lines.
  bool NeedFormat() const;
  void Prepend(RootInlineBox* line);

  gfx::RectF bounds_;
  bool dirty_;
  bool dirty_line_point_;
  int format_counter_;
  std::list<RootInlineBox*> lines_;
  float lines_height_;

  // True if we need to format all lines.
  bool need_format_;
  text::Buffer* const text_buffer_;
  std::unique_ptr<RootInlineBoxCache> text_line_cache_;
  text::Offset view_start_;
  float zoom_;

  DISALLOW_COPY_AND_ASSIGN(LayoutBlockFlow);
};

}  // namespace layout

#endif  // EVITA_LAYOUT_LAYOUT_BLOCK_FLOW_H_
