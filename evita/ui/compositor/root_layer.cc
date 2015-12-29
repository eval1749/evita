// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/compositor/root_layer.h"

#include <dcomp.h>

#include "base/win/scoped_comptr.h"
#include "evita/gfx/rect.h"
#include "evita/ui/compositor/compositor.h"
#include "evita/ui/widget.h"

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// NativeRootLayer
//
class NativeRootLayer final {
 public:
  NativeRootLayer(Widget* widget, RootLayer* layer);
  ~NativeRootLayer() = default;

 private:
  base::win::ScopedComPtr<IDCompositionTarget> composition_target_;

  DISALLOW_COPY_AND_ASSIGN(NativeRootLayer);
};

NativeRootLayer::NativeRootLayer(Widget* widget, RootLayer* layer) {
  const auto  kTopmost = false;
  COM_VERIFY(layer->compositor()->desktop_device()->CreateTargetForHwnd(
      widget->AssociatedHwnd(), kTopmost, composition_target_.Receive()));
  composition_target_->SetRoot(layer->visual());
}

//////////////////////////////////////////////////////////////////////
//
// RootLayer
//
RootLayer::RootLayer(Widget* widget)
    : impl_(new NativeRootLayer(widget, this)) {
  SetBounds(widget->bounds());
}

RootLayer::~RootLayer() {}

}  // namespace ui
