// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_PAINT_ROOT_INLINE_BOX_PAINTER_H_
#define EVITA_PAINT_ROOT_INLINE_BOX_PAINTER_H_

#include "base/macros.h"

namespace gfx {
class Canvas;
}

namespace paint {

class RootInlineBox;

//////////////////////////////////////////////////////////////////////
//
// RootInlineBoxPainter
//
class RootInlineBoxPainter final {
 public:
  explicit RootInlineBoxPainter(const RootInlineBox& root_box);
  ~RootInlineBoxPainter();

  void Paint(gfx::Canvas* canvas);

 private:
  const RootInlineBox& root_box_;

  DISALLOW_COPY_AND_ASSIGN(RootInlineBoxPainter);
};

}  // namespace paint

#endif  // EVITA_PAINT_ROOT_INLINE_BOX_PAINTER_H_
