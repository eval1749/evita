// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_compositor_layer_animation_h)
#define INCLUDE_evita_ui_compositor_layer_animation_h

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
  protected: LayerAnimation();
  public: virtual ~LayerAnimation();

  protected: base::TimeDelta animation_duration() const;

  // Extend |new_layer| from size of |old_layer| then destroy |old_layer|.
  public: static LayerAnimation* CreateExtend(
      Layer* new_layer, std::unique_ptr<Layer> old_layer);

  // Move |new_layer| from |old_origin| on |parent_layer|.
  public: static LayerAnimation* CreateMove(
      Layer* parent_layer, Layer* new_layer, const gfx::PointF& old_origin);

  // Shrink |new_layer| from size of |old_layer| then remove |old_layer|.
  public: static LayerAnimation* CreateShrink(
      Layer* new_layer, std::unique_ptr<Layer> old_layer);

  // Move |new_layer| to |old_layer| on |parent_layer|, then remove |old_layer|.
  public: static LayerAnimation* CreateSlideIn(
      Layer* parent_layer, Layer* new_layer, Layer* old_layer);

  // Move |layer| to |new_origin| then destroy |layer|.
  public: static LayerAnimation* CreateSlideOut(
      std::unique_ptr<Layer> layer, const gfx::PointF& new_origin);

  // Replace |old_layer| by |new_layer|.
  public: static LayerAnimation* CreateSlideReplace(
      Layer* new_layer, std::unique_ptr<Layer> old_layer,
      const gfx::PointF& old_origin);

  DISALLOW_COPY_AND_ASSIGN(LayerAnimation);
};

} // namespace ui

#endif //!defined(INCLUDE_evita_ui_compositor_layer_animation_h)
