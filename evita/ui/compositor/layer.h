// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_compositor_layer_h)
#define INCLUDE_evita_ui_compositor_layer_h

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
class Layer  : private ui::AnimationObserver {
  friend class LayerOwner;

  private: ui::Animatable* animatable_;
  private: gfx::RectF bounds_;
  private: std::list<Layer*> child_layers_;
  private: LayerOwner* owner_;
  private: Layer* parent_layer_;
  private: common::ComPtr<IDCompositionVisual2> visual_;

  public: Layer();
  public: virtual ~Layer();

  public: operator IDCompositionVisual2*() const { return visual_; }

  public: const gfx::RectF& bounds() const { return bounds_; }
  public: Layer* first_child() const;
  public: LayerOwner* owner() { return owner_; }
  public: ui::Layer* parent_layer() const { return parent_layer_; }
  public: IDCompositionVisual2* visual() const { return visual_; }

  public: void AppendLayer(Layer* new_child);
  public: gfx::Canvas* CreateCanvas();
  protected: virtual void DidChangeBounds();
  public: void DidRegisterAnimation(Animatable* animatable);
  public: void FinishAnimation();
  public: void InsertLayer(Layer* new_child, Layer* ref_child);
  public: void RemoveClip();
  public: void RemoveLayer(Layer* old_child);
  public: void SetBounds(const gfx::RectF& new_bounds);
  public: void SetBounds(const gfx::Rect& new_bounds);
  public: void SetClip(const gfx::RectF& bounds);
  public: void SetOrigin(const gfx::PointF& new_origin);

  // AnimationObserver
  private: virtual void DidCancelAnimation(Animatable* animatable) override;
  private: virtual void DidFinishAnimation(Animatable* animatable) override;

  DISALLOW_COPY_AND_ASSIGN(Layer);
};

class HwndLayer : public Layer {
  public: HwndLayer(HWND hwnd);
  public: virtual ~HwndLayer();

  DISALLOW_COPY_AND_ASSIGN(HwndLayer);
};

} // namespace ui

#endif //!defined(INCLUDE_evita_ui_compositor_layer_h)
