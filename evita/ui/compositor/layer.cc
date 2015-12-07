// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/compositor/layer.h"

#include <dcomp.h>
#include <dxgi1_3.h>

#include "base/logging.h"
#include "evita/gfx/canvas.h"
#include "evita/gfx/color_f.h"
#include "evita/gfx/dx_device.h"
#include "evita/gfx/rect_conversions.h"
#include "evita/gfx/swap_chain.h"
#include "evita/ui/animation/animatable.h"
#include "evita/ui/compositor/compositor.h"

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// Layer
//
Layer::Layer(Compositor* compositor)
    : animatable_(nullptr),
      compositor_(compositor),
      owner_(nullptr),
      parent_layer_(nullptr),
      visual_(compositor_->CreateVisual()) {
  COM_VERIFY(visual_->SetBitmapInterpolationMode(
      DCOMPOSITION_BITMAP_INTERPOLATION_MODE_LINEAR));
  COM_VERIFY(visual_->SetBorderMode(DCOMPOSITION_BORDER_MODE_SOFT));
}

Layer::Layer() : Layer(Compositor::instance()) {}

Layer::~Layer() {
  DCHECK(!owner_);
  if (animatable_) {
    animatable_->RemoveObserver(this);
    animatable_->CancelAnimation();
  }
  if (parent_layer_)
    parent_layer_->RemoveLayer(this);
  while (auto const child = first_child()) {
    DCHECK(!child->owner_);
    RemoveLayer(child);
    delete child;
  }
  visual_->SetContent(nullptr);
  visual_->RemoveAllVisuals();
}

Layer* Layer::first_child() const {
  return child_layers_.empty() ? nullptr : child_layers_.front();
}

void Layer::AppendLayer(Layer* new_child) {
  DCHECK_NE(this, new_child->parent_layer_);
  DCHECK_NE(this, new_child);
  DCHECK(!IsDescendantOf(new_child));
  if (auto const old_parent = new_child->parent_layer_)
    old_parent->RemoveLayer(new_child);
  new_child->parent_layer_ = this;
  child_layers_.push_back(new_child);
  auto const is_insert_above = false;
  auto const ref_visual = static_cast<IDCompositionVisual*>(nullptr);
  COM_VERIFY(
      visual_->AddVisual(new_child->visual_, is_insert_above, ref_visual));
  compositor_->NeedCommit();
}

gfx::Canvas* Layer::CreateCanvas() {
  DCHECK(!bounds_.empty());
  DCHECK_EQ(bounds_, gfx::RectF(gfx::ToEnclosingRect(bounds_)));
  return new gfx::Canvas(this);
}

void Layer::DidChangeBounds() {
  compositor_->NeedCommit();
}

void Layer::DidRegisterAnimation(ui::Animatable* animatable) {
  FinishAnimation();
  animatable_ = animatable;
  animatable_->AddObserver(this);
}

void Layer::FinishAnimation() {
  auto const animatable = animatable_;
  if (!animatable)
    return;
  animatable_ = nullptr;
  animatable->RemoveObserver(this);
  animatable->FinishAnimation();
}

void Layer::InsertLayer(Layer* new_child, Layer* ref_child) {
  if (!ref_child) {
    AppendLayer(new_child);
    return;
  }
  DCHECK_NE(this, new_child);
  DCHECK(!IsDescendantOf(new_child));
  DCHECK_EQ(this, ref_child->parent_layer_);
  if (auto const old_parent = new_child->parent_layer_)
    old_parent->RemoveLayer(new_child);
  new_child->parent_layer_ = this;
  auto const it =
      std::find(child_layers_.begin(), child_layers_.end(), ref_child);
  DCHECK(it != child_layers_.end());
  child_layers_.insert(it, new_child);
  auto const is_insert_above = false;
  COM_VERIFY(visual_->AddVisual(new_child->visual_, is_insert_above,
                                ref_child->visual_));
  compositor_->NeedCommit();
}

bool Layer::IsDescendantOf(const Layer* other) const {
  for (auto runner = parent_layer_; runner; runner = runner->parent_layer_) {
    if (runner == other)
      return true;
  }
  return false;
}

void Layer::RemoveClip() {
  visual_->SetClip(nullptr);
  compositor_->NeedCommit();
}

void Layer::RemoveLayer(Layer* old_layer) {
  DCHECK_EQ(old_layer->parent_layer_, this);
  auto const it =
      std::find(child_layers_.begin(), child_layers_.end(), old_layer);
  DCHECK(it != child_layers_.end());
  child_layers_.erase(it);
  visual_->RemoveVisual(old_layer->visual_);
  old_layer->parent_layer_ = nullptr;
  compositor_->NeedCommit();
}

void Layer::SetBounds(const gfx::RectF& new_bounds) {
  DCHECK(!new_bounds.empty());
  DCHECK_EQ(new_bounds, gfx::RectF(gfx::ToEnclosingRect(new_bounds)));
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

void Layer::SetClip(const gfx::RectF& bounds) {
  DCHECK_EQ(bounds, gfx::RectF(gfx::ToEnclosingRect(bounds)));
  visual_->SetClip(bounds);
  compositor_->NeedCommit();
}

void Layer::SetOrigin(const gfx::PointF& new_origin) {
  SetBounds(gfx::RectF(new_origin, bounds_.size()));
}

// AnimationObserver
void Layer::DidCancelAnimation(Animatable* animatable) {
  DCHECK_EQ(animatable_, animatable);
  animatable_->RemoveObserver(this);
  animatable_ = nullptr;
}

void Layer::DidFinishAnimation(Animatable* animatable) {
  DCHECK_EQ(animatable_, animatable);
  animatable_->RemoveObserver(this);
  animatable_ = nullptr;
}

// gfx::CanvasOwner
std::unique_ptr<gfx::SwapChain> Layer::CreateSwapChain() {
  auto const device = compositor_->device();
  auto swap_chain = gfx::SwapChain::CreateForComposition(device, bounds());
  COM_VERIFY(visual_->SetContent(swap_chain->swap_chain()));
  compositor_->NeedCommit();
  return swap_chain;
}

}  // namespace ui
