// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_PAINT_ROOT_INLINE_BOX_PAINTER_H_
#define EVITA_PAINT_ROOT_INLINE_BOX_PAINTER_H_

#include "base/macros.h"

namespace gfx {
class Canvas;
}

namespace views {
namespace rendering {
class RootInlineBox;
}
}

namespace paint {

//////////////////////////////////////////////////////////////////////
//
// RootInlineBoxPainter
//
class RootInlineBoxPainter final {
  using RootInlineBox = views::rendering::RootInlineBox;

 public:
  explicit RootInlineBoxPainter(const RootInlineBox& root_inline_box);
  ~RootInlineBoxPainter();

  void Paint(gfx::Canvas* canvas);

 private:
  const RootInlineBox& root_inline_box_;

  DISALLOW_COPY_AND_ASSIGN(RootInlineBoxPainter);
};

}  // namespace paint

#endif  // EVITA_PAINT_ROOT_INLINE_BOX_PAINTER_H_
