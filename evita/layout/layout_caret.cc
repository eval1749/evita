// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/layout/layout_caret.h"

namespace layout {

LayoutCaret::LayoutCaret(State state, const gfx::RectF& bounds)
    : bounds_(bounds), state_(state) {
  if (is_none()) {
    DCHECK(bounds_.empty());
    return;
  }
  DCHECK(!bounds_.empty());
}

LayoutCaret::~LayoutCaret() {}

}  // namespace layout
