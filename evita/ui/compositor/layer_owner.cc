// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <utility>

#include "evita/ui/compositor/layer_owner.h"

#include "base/logging.h"
#include "evita/ui/compositor/layer.h"
#include "evita/ui/compositor/layer_owner_delegate.h"

namespace ui {

LayerOwner::LayerOwner() : delegate_(nullptr), layer_(nullptr) {
}

LayerOwner::~LayerOwner() {
}

void LayerOwner::set_layer_owner_delegate(LayerOwnerDelegate* delegate) {
  DCHECK(!delegate_);
  delegate_ = delegate;
}

std::unique_ptr<Layer> LayerOwner::AcquireLayer() {
  if (owned_layer_)
    owned_layer_->owner_ = nullptr;
  return std::move(owned_layer_);
}

void LayerOwner::DestroyLayer() {
  if (!OwnsLayer())
    return;
  layer_ = nullptr;
  owned_layer_.reset();
}

bool LayerOwner::OwnsLayer() const {
  return !!owned_layer_;
}

std::unique_ptr<Layer> LayerOwner::RecreateLayer() {
  auto old_layer = AcquireLayer();
  if (!old_layer)
    return std::move(old_layer);

  auto const new_layer = new Layer();
  new_layer->SetBounds(old_layer->bounds());
  SetLayer(new_layer);
  if (auto const parent = old_layer->parent_layer()) {
    parent->InsertLayer(new_layer, old_layer.get());
  }
  while (auto const child = old_layer->first_child()) {
    new_layer->AppendLayer(child);
  }
  if (delegate_)
    delegate_->DidRecreateLayer(old_layer.get());
  return std::move(old_layer);
}

void LayerOwner::SetLayer(Layer* layer) {
  DCHECK(!OwnsLayer());
  owned_layer_.reset(layer);
  layer_ = layer;
  layer_->owner_ = this;
}

}  // namespace ui
