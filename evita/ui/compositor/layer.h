// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_COMPOSITOR_LAYER_H_
#define EVITA_UI_COMPOSITOR_LAYER_H_

#include <list>

#include "base/basictypes.h"
#include "common/win/scoped_comptr.h"
#include "evita/gfx/rect.h"
#include "evita/gfx/rect_f.h"
#include "evita/ui/animation/animation_observer.h"

interface IDCompositionVisual2;

namespace gfx {
class Canvas;
}

namespace ui {

class Animatable;
class LayerOwner;

//////////////////////////////////////////////////////////////////////
//
// Layer
//
class Layer : private ui::AnimationObserver {
 public:
  Layer();
  ~Layer() override;

  operator IDCompositionVisual2*() const { return visual_; }

  const gfx::RectF& bounds() const { return bounds_; }
  Layer* first_child() const;
  gfx::PointF origin() const { return bounds_.origin(); }
  LayerOwner* owner() { return owner_; }
  ui::Layer* parent_layer() const { return parent_layer_; }
  IDCompositionVisual2* visual() const { return visual_; }

  void AppendLayer(Layer* new_child);
  gfx::Canvas* CreateCanvas();
  void DidRegisterAnimation(Animatable* animatable);
  void FinishAnimation();
  void InsertLayer(Layer* new_child, Layer* ref_child);
  void RemoveClip();
  void RemoveLayer(Layer* old_child);
  void SetBounds(const gfx::RectF& new_bounds);
  void SetBounds(const gfx::Rect& new_bounds);
  void SetClip(const gfx::RectF& bounds);
  void SetOrigin(const gfx::PointF& new_origin);

 protected:
  void DidChangeBounds();

 private:
  friend class LayerOwner;

  bool IsDescendantOf(const Layer* other) const;

  // AnimationObserver
  void DidCancelAnimation(Animatable* animatable) override;
  void DidFinishAnimation(Animatable* animatable) override;

  ui::Animatable* animatable_;
  gfx::RectF bounds_;
  std::list<Layer*> child_layers_;
  LayerOwner* owner_;
  Layer* parent_layer_;
  common::ComPtr<IDCompositionVisual2> visual_;

  DISALLOW_COPY_AND_ASSIGN(Layer);
};

}  // namespace ui

#endif  // EVITA_UI_COMPOSITOR_LAYER_H_
