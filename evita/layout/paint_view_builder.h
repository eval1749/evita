// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_LAYOUT_PAINT_VIEW_BUILDER_H_
#define EVITA_LAYOUT_PAINT_VIEW_BUILDER_H_

#include <memory>

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/time/time.h"
#include "evita/gfx/rect_f.h"

namespace paint {
class Caret;
class View;
}

namespace text {
class Buffer;
}

namespace layout {

class BlockFlow;
class RenderStyle;
class TextSelectionModel;

//////////////////////////////////////////////////////////////////////
//
// PaintViewBuilder
//
class PaintViewBuilder final {
  using CaretDisplayItem = paint::Caret;

 public:
  PaintViewBuilder();
  ~PaintViewBuilder();

  scoped_refptr<paint::View> Build(const BlockFlow& block,
                                   const TextSelectionModel& selection,
                                   const CaretDisplayItem& caret);

 private:
  DISALLOW_COPY_AND_ASSIGN(PaintViewBuilder);
};

}  // namespace layout

#endif  // EVITA_LAYOUT_PAINT_VIEW_BUILDER_H_
