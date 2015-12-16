// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_LAYOUT_LINE_ROOT_INLINE_BOX_CACHE_H_
#define EVITA_LAYOUT_LINE_ROOT_INLINE_BOX_CACHE_H_

#include <map>

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "evita/gfx/rect_f.h"
#include "evita/text/offset.h"

namespace text {
class Buffer;
}

namespace layout {

class RootInlineBox;

//////////////////////////////////////////////////////////////////////
//
// RootInlineBoxCache
//
class RootInlineBoxCache final {
 public:
  explicit RootInlineBoxCache(const text::Buffer* buffer);
  ~RootInlineBoxCache();

  void DidChangeBuffer(text::Offset offset);
  scoped_refptr<RootInlineBox> FindLine(text::Offset text_offset) const;
  void Invalidate(const gfx::RectF& bounds, float zoom);
  bool IsDirty(const gfx::RectF& bounds, float zoom) const;
  void Register(scoped_refptr<RootInlineBox> line);

 private:
  bool IsAfterNewline(const RootInlineBox* text_line) const;
  bool IsEndWithNewline(const RootInlineBox* text_line) const;
  void RemoveDirtyLines();
  void RemoveAllLines();

  gfx::RectF bounds_;
  const text::Buffer* const buffer_;
  text::Offset dirty_start_;
  std::map<text::Offset, scoped_refptr<RootInlineBox>> lines_;
  float zoom_;

  DISALLOW_COPY_AND_ASSIGN(RootInlineBoxCache);
};

}  // namespace layout

#endif  // EVITA_LAYOUT_LINE_ROOT_INLINE_BOX_CACHE_H_
