// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/text/text_view_caret.h"

#include "evita/gfx/canvas.h"

namespace views {

TextViewCaret::TextViewCaret(ui::CaretOwner* owner) : ui::Caret(owner) {}

TextViewCaret::~TextViewCaret() {}

void TextViewCaret::Paint(gfx::Canvas* canvas, const gfx::RectF& bounds) {
  if (visible()) {
    gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, bounds);
    canvas->AddDirtyRect(bounds);
    canvas->Clear(gfx::ColorF::Black);
    return;
  }
  if (!canvas->screen_bitmap())
    return;
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, bounds);
  canvas->AddDirtyRect(bounds);
  canvas->RestoreScreenImage(bounds);
}

}  // namespace views
