// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_COMPOSITOR_ROOT_LAYER_H_
#define EVITA_UI_COMPOSITOR_ROOT_LAYER_H_

#include <memory>

#include "evita/ui/compositor/layer.h"

namespace ui {

class NativeRootLayer;
class Widget;

//////////////////////////////////////////////////////////////////////
//
// RootLayer represents a topmost layer of DirectComposition.
//
class RootLayer final : public Layer {
 public:
  explicit RootLayer(Widget* widget);
  ~RootLayer() final;

 private:
  std::unique_ptr<NativeRootLayer> impl_;

  DISALLOW_COPY_AND_ASSIGN(RootLayer);
};

}  // namespace ui

#endif  // EVITA_UI_COMPOSITOR_ROOT_LAYER_H_
