// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/compositor/layer.h"

#include <dcomp.h>
#include <dxgi1_3.h>

#include "base/logging.h"
#include "evita/gfx/canvas.h"
#include "evita/gfx/color_f.h"
#include "evita/gfx/swap_chain.h"
#include "evita/ui/animation/animatable.h"
#include "evita/ui/compositor/compositor.h"
#include "evita/ui/compositor/canvas_for_layer.h"

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// Layer
//
Layer::Layer()
    : animatable_(nullptr), owner_(nullptr), parent_layer_(nullptr),
      visual_(Compositor::instance()->CreateVisual()) {
  COM_VERIFY(visual_->SetBitmapInterpolationMode(
      DCOMPOSITION_BITMAP_INTERPOLATION_MODE_LINEAR));
  COM_VERIFY(visual_->SetBorderMode(DCOMPOSITION_BORDER_MODE_SOFT));

  common::ComPtr<IDCompositionVisualDebug> debug_visual;
  COM_VERIFY(debug_visual.QueryFrom(visual_));
  //COM_VERIFY(debug_visual->EnableHeatMap(gfx::ColorF(255, 255, 0, 0.1)));
  // Node: EnableRedrawRegions() makes too many color changes.
  // COM_VERIFY(debug_visual->EnableRedrawRegions());
}

Layer::~Layer() {
  if (animatable_)
    animatable_->CancelAnimation();
  visual_->SetContent(nullptr);
  visual_->RemoveAllVisuals();
}

void Layer::AppendLayer(Layer* new_child) {
  DCHECK_NE(this, new_child->parent_layer_);
  if (auto const old_parent = new_child->parent_layer_)
    old_parent->RemoveLayer(new_child);
  new_child->parent_layer_ = this;
  child_layers_.insert(new_child);
  auto const is_insert_above = false;
  auto const ref_visual = static_cast<IDCompositionVisual*>(nullptr);
  COM_VERIFY(visual_->AddVisual(new_child->visual_, is_insert_above,
                                ref_visual));
  Compositor::instance()->NeedCommit();
}

gfx::Canvas* Layer::CreateCanvas() {
  DCHECK(!bounds_.empty());
  auto canvas = new gfx::CanvasForLayer(this);
  COM_VERIFY(visual_->SetContent(canvas->swap_chain()->swap_chain()));
  return canvas;
}

void Layer::DidChangeBounds() {
  Compositor::instance()->NeedCommit();
}

void Layer::EndAnimation() {
  DCHECK(animatable_);
  animatable_->RemoveObserver(this);
  animatable_ = nullptr;
}

void Layer::RemoveLayer(Layer* old_layer) {
  DCHECK_EQ(old_layer->parent_layer_, this);
  child_layers_.erase(old_layer);
  visual_->RemoveVisual(old_layer->visual_);
  old_layer->parent_layer_ = nullptr;
  Compositor::instance()->NeedCommit();
}

void Layer::SetBounds(const gfx::RectF& new_bounds) {
  DCHECK(!new_bounds.empty());
  auto changed = false;
  if (bounds_.left != new_bounds.left) {
    COM_VERIFY(visual_->SetOffsetX(new_bounds.left));
    bounds_.set_origin(gfx::PointF(new_bounds.left, bounds_.top));
    changed = true;
  }
  if (bounds_.top != new_bounds.top) {
    COM_VERIFY(visual_->SetOffsetY(new_bounds.top));
    bounds_.set_origin(gfx::PointF(bounds_.left, new_bounds.top));
    changed = true;
  }

  if (bounds_.size() != new_bounds.size()) {
    bounds_.set_size(new_bounds.size());
    changed = true;
  }

  if (!changed)
    return;

  DidChangeBounds();
}

void Layer::SetBounds(const gfx::Rect& new_bounds) {
  SetBounds(gfx::RectF(new_bounds));
}

void Layer::SetOrigin(const gfx::PointF& new_origin) {
  SetBounds(gfx::RectF(new_origin, bounds_.size()));
}

void Layer::StartAnimation(ui::Animatable* animatable) {
  DCHECK(!animatable_);
  animatable_ = animatable;
  animatable_->AddObserver(this);
}

// AnimationObserver
void Layer::DidCancelAnimation(Animatable* animatable) {
  DCHECK_EQ(animatable_, animatable);
  animatable_->RemoveObserver(this);
  animatable_ = nullptr;
}

//////////////////////////////////////////////////////////////////////
//
// HwndLayer
//
HwndLayer::HwndLayer(HWND hwnd) {
  ::SetLastError(0);
  ::SetWindowLong(hwnd, GWL_EXSTYLE,
                  ::GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
  if (auto const last_error = ::GetLastError()) {
    DVLOG(ERROR) << "SetWindowLong GWL_EXSTYLE err=" << last_error;
    NOTREACHED();
  }
  DVLOG(0) << "ex_style=0x" << std::hex << ::GetWindowLong(hwnd, GWL_EXSTYLE);
  //DCHECK(::GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_LAYERED);
  common::ComPtr<IUnknown> surface;
  COM_VERIFY(Compositor::instance()->device()->
      CreateSurfaceFromHwnd(hwnd, &surface));
  visual()->SetContent(surface);
  RECT bounds;
  ::GetClientRect(hwnd, &bounds);
  visual()->SetOffsetX(static_cast<float>(bounds.left));
  visual()->SetOffsetY(static_cast<float>(bounds.top));
}

HwndLayer::~HwndLayer() {
}

}  // namespace ui
