// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_PAINT_ROOT_INLINE_BOX_PAINTER_H_
#define EVITA_PAINT_ROOT_INLINE_BOX_PAINTER_H_

#include "base/macros.h"
#include "evita/views/text/root_inline_box.h"

namespace gfx {
class Canvas;
}

namespace paint {

//////////////////////////////////////////////////////////////////////
//
// RootInlineBoxPainter
//
class RootInlineBoxPainter final {
 public:
  explicit RootInlineBoxPainter(gfx::Canvas* canvas);
  ~RootInlineBoxPainter();

  void Paint(const views::rendering::RootInlineBox& inline_box);

 private:
  gfx::Canvas* const canvas_;

  DISALLOW_COPY_AND_ASSIGN(RootInlineBoxPainter);
};

}  // namespace paint

#endif  // EVITA_PAINT_ROOT_INLINE_BOX_PAINTER_H_
