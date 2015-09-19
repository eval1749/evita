// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_COMPOSITOR_ROOT_LAYER_H_
#define EVITA_UI_COMPOSITOR_ROOT_LAYER_H_

#include "evita/ui/compositor/layer.h"

interface IDCompositionTarget;

namespace ui {

class Widget;

class RootLayer final : public Layer {
 public:
  explicit RootLayer(Widget* widget);
  ~RootLayer();

 private:
  common::ComPtr<IDCompositionTarget> composition_target_;

  DISALLOW_COPY_AND_ASSIGN(RootLayer);
};

}  // namespace ui

#endif  // EVITA_UI_COMPOSITOR_ROOT_LAYER_H_
