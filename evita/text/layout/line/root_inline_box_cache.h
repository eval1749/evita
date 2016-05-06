// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_LAYOUT_LINE_ROOT_INLINE_BOX_CACHE_H_
#define EVITA_TEXT_LAYOUT_LINE_ROOT_INLINE_BOX_CACHE_H_

#include <map>
#include <memory>

#include "base/macros.h"
#include "evita/gfx/rect_f.h"
#include "evita/text/models/buffer_mutation_observer.h"
#include "evita/text/models/marker_set_observer.h"
#include "evita/text/models/offset.h"

namespace text {
class Buffer;
class MarkerSet;
class StaticRange;
}

namespace layout {

class RootInlineBox;

//////////////////////////////////////////////////////////////////////
//
// RootInlineBoxCache
//
class RootInlineBoxCache final : public text::BufferMutationObserver,
                                 public text::MarkerSetObserver {
 public:
  RootInlineBoxCache(const text::Buffer& buffer,
                     const text::MarkerSet& markers);
  ~RootInlineBoxCache();

  // Returns |RootInlineBox| containing |text_offset|.
  RootInlineBox* FindLine(text::Offset text_offset) const;
  void Invalidate(const gfx::RectF& bounds, float zoom);
  bool IsDirty(const gfx::RectF& bounds, float zoom) const;
  RootInlineBox* Register(std::unique_ptr<RootInlineBox> line);

 private:
  RootInlineBox* Insert(std::unique_ptr<RootInlineBox> line);
  void RelocateLines(text::Offset offset, text::OffsetDelta delta);
  void RemoveOverwapLines(const text::StaticRange& range);

  // text::BufferMutationObserver
  void DidChangeStyle(const text::StaticRange& range) final;
  void DidDeleteAt(const text::StaticRange& range) final;
  void DidInsertBefore(const text::StaticRange& range) final;

  // text::MarkerSetObserver
  void DidChangeMarker(text::Offset start, text::Offset end) final;

  gfx::RectF bounds_;
  const text::Buffer& buffer_;
  // |lines_| keeps |RootInlineBox| indexed by end offset and manages
  // life time of |RootInlineBox|.
  std::map<text::Offset, std::unique_ptr<RootInlineBox>> lines_;
  const text::MarkerSet& markers_;
  float zoom_ = 0.0f;

  DISALLOW_COPY_AND_ASSIGN(RootInlineBoxCache);
};

}  // namespace layout

#endif  // EVITA_TEXT_LAYOUT_LINE_ROOT_INLINE_BOX_CACHE_H_
