// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/compositor/root_layer.h"

#include <dcomp.h>

#include "evita/gfx/rect.h"
#include "evita/ui/compositor/compositor.h"
#include "evita/ui/widget.h"

namespace ui {

RootLayer::RootLayer(Widget* widget) {
  COM_VERIFY(Compositor::instance()->device()->
      CreateTargetForHwnd(widget->AssociatedHwnd(), false,
                          &composition_target_));
  composition_target_->SetRoot(visual());
  SetBounds(widget->bounds());
}

RootLayer::~RootLayer() {
}

}  // namespace ui
