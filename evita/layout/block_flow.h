// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_LAYOUT_BLOCK_FLOW_H_
#define EVITA_LAYOUT_BLOCK_FLOW_H_

#include <list>
#include <memory>

#include "base/memory/ref_counted.h"
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
// BlockFlow
//
class BlockFlow final {
 public:
  explicit BlockFlow(text::Buffer* buffer);
  ~BlockFlow();

  const gfx::RectF& bounds() const { return bounds_; }
  bool dirty() const { return dirty_; }
  int version() const { return version_; }
  const std::list<scoped_refptr<RootInlineBox>>& lines() const {
    return lines_;
  }
  gfx::PointF origin() const { return bounds_.origin(); }
  const text::Buffer& text_buffer() const { return *text_buffer_; }
  text::Offset text_end() const;
  text::Offset text_start() const;

  // Returns end of line offset containing |text_offset|.
  text::Offset ComputeEndOfLine(text::Offset text_offset);
  // Returns start of line offset containing |text_offset|.
  text::Offset ComputeStartOfLine(text::Offset text_offset);
  text::Offset ComputeVisibleEnd() const;

  // Callback for buffer mutation observer
  void DidChangeStyle(text::Offset offset, text::OffsetDelta length);
  void DidDeleteAt(text::Offset offset, text::OffsetDelta length);
  void DidInsertBefore(text::Offset offset, text::OffsetDelta length);

  void Format(text::Offset text_offset);
  // Returns true if text format is taken place.
  bool FormatIfNeeded();
  text::Offset HitTestPoint(gfx::PointF point) const;
  gfx::RectF HitTestTextPosition(text::Offset text_offset) const;
  bool IsFullyVisibleTextPosition(text::Offset text_offset) const;
  text::Offset MapPointXToOffset(text::Offset text_offset, float point_x);
  // Returns true if we need to format all lines.
  bool NeedsFormat() const;
  // Returns true if this |BlockFlow| is modified.
  bool ScrollDown();
  // Returns true if this |BlockFlow| is modified.
  bool ScrollToPosition(text::Offset offset);
  // Returns true if this |BlockFlow| is modified.
  bool ScrollUp();
  void SetBounds(const gfx::RectF& new_bounds);
  void SetZoom(float new_zoom);
  bool ShouldFormat() const;

 private:
  void Append(scoped_refptr<RootInlineBox> line);
  // Returns true if discarded the first line.
  bool DiscardFirstLine();
  // Returns true if discarded the last line.
  bool DiscardLastLine();
  void EnsureLinePoints();
  void EnsureTextLineCache();
  RootInlineBox* FindLineContainng(text::Offset offset) const;
  scoped_refptr<RootInlineBox> FormatLine(TextFormatter* formatter);
  bool IsShowEndOfDocument() const;

  void Prepend(scoped_refptr<RootInlineBox> line);

  gfx::RectF bounds_;
  bool dirty_;
  bool dirty_line_point_;
  std::list<scoped_refptr<RootInlineBox>> lines_;
  float lines_height_;

  // True if we need to format all lines.
  bool needs_format_;
  text::Buffer* const text_buffer_;
  std::unique_ptr<RootInlineBoxCache> text_line_cache_;
  int version_;
  text::Offset view_start_;
  float zoom_;

  DISALLOW_COPY_AND_ASSIGN(BlockFlow);
};

}  // namespace layout

#endif  // EVITA_LAYOUT_BLOCK_FLOW_H_
