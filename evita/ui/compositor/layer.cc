// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/compositor/layer.h"

#include <dcomp.h>

#include "evita/ui/compositor/compositor.h"

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// Layer
//
Layer::Layer(Compositor* compositor)
    : compositor_(compositor), is_active_(false),
      visual_(compositor->CreateVisual()) {
  COM_VERIFY(visual_->SetBitmapInterpolationMode(
      DCOMPOSITION_BITMAP_INTERPOLATION_MODE_LINEAR));
  COM_VERIFY(visual_->SetBorderMode(DCOMPOSITION_BORDER_MODE_SOFT));

  common::ComPtr<IDCompositionVisualDebug> debug_visual;
  COM_VERIFY(debug_visual.QueryFrom(visual_));
  // TODO(eval1749) It seems heat map is painted with alpha=1.0.
  //COM_VERIFY(debug_visual->EnableHeatMap(gfx::ColorF(255, 255, 0, 0.1)));
  //COM_VERIFY(debug_visual->EnableRedrawRegions());
}

Layer::~Layer() {
  visual_->SetContent(nullptr);
  visual_->RemoveAllVisuals();
}

void Layer::AppendChild(Layer* new_child) {
  child_layers_.push_back(new_child);
  COM_VERIFY(visual_->AddVisual(new_child->visual_, true, nullptr));
}

void Layer::DidActive() {
  if (is_active_)
    return;
  is_active_ = true;
  for (auto const child : child_layers_) {
    child->DidActive();
  }
}

void Layer::DidChangeBounds() {
}

void Layer::DidInactive() {
  if (!is_active_)
    return;
  is_active_ = false;
  for (auto const child : child_layers_) {
    child->DidInactive();
  }
}

bool Layer::DoAnimate(base::TimeTicks tick_count) {
  auto animated = false;
  for (auto const child : child_layers_) {
    animated |= child->DoAnimate(tick_count);
  }
  return animated;
}

void Layer::SetBounds(const gfx::RectF& new_bounds) {
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

}  // namespace ui
