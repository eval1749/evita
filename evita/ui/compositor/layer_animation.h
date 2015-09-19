// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_COMPOSITOR_LAYER_ANIMATION_H_
#define EVITA_UI_COMPOSITOR_LAYER_ANIMATION_H_

#include <memory>

#include "evita/gfx/point_f.h"
#include "evita/ui/animation/animatable.h"

namespace ui {

class Layer;

//////////////////////////////////////////////////////////////////////
//
// LayerAnimation
//
class LayerAnimation : public Animatable {
 public:
  virtual ~LayerAnimation();

  // Extend |new_layer| from size of |old_layer| then destroy |old_layer|.
  static LayerAnimation* CreateExtend(Layer* new_layer,
                                      std::unique_ptr<Layer> old_layer);

  // Move |new_layer| from |old_origin| on |parent_layer|.
  static LayerAnimation* CreateMove(Layer* parent_layer,
                                    Layer* new_layer,
                                    const gfx::PointF& old_origin);

  // Shrink |old_layer| to |new_layer| then remove |old_layer|.
  static LayerAnimation* CreateShrink(Layer* new_layer,
                                      std::unique_ptr<Layer> old_layer);

  static LayerAnimation* CreateSimple(Layer* new_layer,
                                      std::unique_ptr<Layer> old_layer);

  // Move |new_layer| to |old_layer| on |parent_layer|, then remove |old_layer|.
  static LayerAnimation* CreateSlideIn(Layer* parent_layer,
                                       Layer* new_layer,
                                       Layer* old_layer);

  // Move |layer| to |new_origin| then destroy |layer|.
  static LayerAnimation* CreateSlideOut(std::unique_ptr<Layer> layer,
                                        const gfx::PointF& new_origin);

  // Replace |old_layer| by |new_layer|.
  static LayerAnimation* CreateSlideReplace(Layer* new_layer,
                                            std::unique_ptr<Layer> old_layer,
                                            const gfx::PointF& old_origin);

 protected:
  LayerAnimation();

  base::TimeDelta animation_duration() const;

 private:
  DISALLOW_COPY_AND_ASSIGN(LayerAnimation);
};

}  // namespace ui

#endif  // EVITA_UI_COMPOSITOR_LAYER_ANIMATION_H_
