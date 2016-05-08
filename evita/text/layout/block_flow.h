// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_LAYOUT_BLOCK_FLOW_H_
#define EVITA_TEXT_LAYOUT_BLOCK_FLOW_H_

#include <list>
#include <memory>

#include "base/memory/ref_counted.h"
#include "evita/gfx/rect_f.h"
#include "evita/text/models/buffer_mutation_observer.h"
#include "evita/text/models/marker_set_observer.h"
#include "evita/text/models/offset.h"
#include "evita/text/style/computed_style.h"

namespace text {
class Buffer;
class MarkerSet;
class StaticRange;
}

namespace layout {

class TextFormatter;
class RootInlineBox;
class RootInlineBoxCache;

//////////////////////////////////////////////////////////////////////
//
// BlockFlow
//
class BlockFlow final : public text::BufferMutationObserver,
                        public text::MarkerSetObserver {
 public:
  BlockFlow(const text::Buffer& buffer, const text::MarkerSet& markers);
  ~BlockFlow();

  const gfx::RectF& bounds() const { return bounds_; }
  int version() const { return version_; }
  const std::list<RootInlineBox*>& lines() const { return lines_; }
  gfx::PointF origin() const { return bounds_.origin(); }
  const text::Buffer& text_buffer() const { return text_buffer_; }
  text::Offset text_end() const;
  text::Offset text_start() const;
  float zoom() const { return zoom_; }

  // Returns end of line offset containing |text_offset|.
  text::Offset ComputeEndOfLine(text::Offset text_offset);
  // Returns start of line offset containing |text_offset|.
  text::Offset ComputeStartOfLine(text::Offset text_offset);
  text::Offset ComputeVisibleEnd() const;

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
  void Append(RootInlineBox* line);
  // Returns true if discarded the first line.
  bool DiscardFirstLine();
  // Returns true if discarded the last line.
  bool DiscardLastLine();
  void EnsureLinePoints();
  void EnsureTextLineCache();
  RootInlineBox* FindLineContainng(text::Offset offset) const;
  RootInlineBox* FormatLine(TextFormatter* formatter);
  bool IsShowEndOfDocument() const;
  void MarkDirty();
  void Prepend(RootInlineBox* line);

  // text::BufferMutationObserver
  void DidChangeStyle(const text::StaticRange& range) final;
  void DidDeleteAt(const text::StaticRange& range) final;
  void DidInsertBefore(const text::StaticRange& range) final;

  // text::MarkerSetObserver
  void DidChangeMarker(const text::StaticRange& range) final;

  gfx::RectF bounds_;
  bool dirty_line_point_ = true;
  std::list<RootInlineBox*> lines_;
  float lines_height_ = 0.0f;
  const text::MarkerSet& markers_;
  const text::Buffer& text_buffer_;
  std::unique_ptr<RootInlineBoxCache> text_line_cache_;
  int version_ = 0;
  text::Offset view_start_;
  float zoom_ = 1.0f;

  DISALLOW_COPY_AND_ASSIGN(BlockFlow);
};

}  // namespace layout

#endif  // EVITA_TEXT_LAYOUT_BLOCK_FLOW_H_
