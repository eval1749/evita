// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_compositor_root_layer_h)
#define INCLUDE_evita_ui_compositor_root_layer_h

#include "evita/ui/compositor/layer.h"

interface IDCompositionTarget;

namespace ui {

class Widget;

class RootLayer : public Layer {
  private: common::ComPtr<IDCompositionTarget> composition_target_;

  public: RootLayer(Widget* widget);
  public: ~RootLayer();

  DISALLOW_COPY_AND_ASSIGN(RootLayer);
};

} // namespace ui

#endif //!defined(INCLUDE_evita_ui_compositor_root_layer_h)
