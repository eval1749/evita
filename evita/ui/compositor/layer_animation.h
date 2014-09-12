// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_compositor_layer_animation_h)
#define INCLUDE_evita_ui_compositor_layer_animation_h

#include "evita/ui/animation/animatable.h"

namespace ui {

class Layer;

//////////////////////////////////////////////////////////////////////
//
// LayerAnimation
//
class LayerAnimation : public Animatable {
  protected: LayerAnimation();
  public: virtual ~LayerAnimation();

  protected: base::TimeDelta animation_duration() const;

  public: static LayerAnimation* CreateSlideInFromLeft(ui::Layer* left_layer,
                                                       ui::Layer* right_layer);

  DISALLOW_COPY_AND_ASSIGN(LayerAnimation);
};

} // namespace ui

#endif //!defined(INCLUDE_evita_ui_compositor_layer_animation_h)
