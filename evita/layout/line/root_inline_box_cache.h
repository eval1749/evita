// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_LAYOUT_LINE_ROOT_INLINE_BOX_CACHE_H_
#define EVITA_LAYOUT_LINE_ROOT_INLINE_BOX_CACHE_H_

#include <map>
#include <memory>

#include "base/macros.h"
#include "evita/gfx/rect_f.h"
#include "evita/text/buffer_mutation_observer.h"
#include "evita/text/offset.h"

namespace text {
class Buffer;
class StaticRange;
}

namespace layout {

class RootInlineBox;

//////////////////////////////////////////////////////////////////////
//
// RootInlineBoxCache
//
class RootInlineBoxCache final : public text::BufferMutationObserver {
 public:
  explicit RootInlineBoxCache(const text::Buffer& buffer);
  ~RootInlineBoxCache();

  RootInlineBox* FindLine(text::Offset text_offset) const;
  void Invalidate(const gfx::RectF& bounds, float zoom);
  bool IsDirty(const gfx::RectF& bounds, float zoom) const;
  RootInlineBox* Register(std::unique_ptr<RootInlineBox> line);

 private:
  bool IsAfterNewline(const RootInlineBox* text_line) const;
  bool IsEndWithNewline(const RootInlineBox* text_line) const;
  void RelocateLines(text::Offset offset, text::OffsetDelta delta);
  void RemoveOverwapLines(const text::StaticRange& range);

  // text::BufferMutationObserver
  void DidChangeStyle(const text::StaticRange& range) final;
  void DidDeleteAt(const text::StaticRange& range) final;
  void DidInsertBefore(const text::StaticRange& range) final;

  gfx::RectF bounds_;
  const text::Buffer& buffer_;
  std::map<text::Offset, std::unique_ptr<RootInlineBox>> lines_;
  float zoom_;

  DISALLOW_COPY_AND_ASSIGN(RootInlineBoxCache);
};

}  // namespace layout

#endif  // EVITA_LAYOUT_LINE_ROOT_INLINE_BOX_CACHE_H_
