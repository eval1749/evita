// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <utility>

#include "evita/ui/compositor/layer_owner.h"

#include "base/logging.h"
#include "evita/ui/compositor/layer.h"
#include "evita/ui/compositor/layer_owner_delegate.h"

namespace ui {

LayerOwner::LayerOwner() : delegate_(nullptr), layer_(nullptr) {}

LayerOwner::~LayerOwner() {}

void LayerOwner::set_layer_owner_delegate(LayerOwnerDelegate* delegate) {
  DCHECK(!delegate_);
  delegate_ = delegate;
}

std::unique_ptr<Layer> LayerOwner::AcquireLayerTree() {
  return std::unique_ptr<Layer>(DetachLayerTree());
}

void LayerOwner::DestroyLayer() {
  if (!OwnsLayer())
    return;
  layer_->owner_ = nullptr;
  layer_ = nullptr;
  owned_layer_.reset();
}

Layer* LayerOwner::DetachLayerTree() {
  DCHECK(OwnsLayer());
  for (auto child_layer : layer_->child_layers_)
    child_layer->owner()->DetachLayerTree();
  layer_->owner_ = nullptr;
  return owned_layer_.release();
}

bool LayerOwner::OwnsLayer() const {
  return !!owned_layer_;
}

std::unique_ptr<Layer> LayerOwner::RecreateLayer() {
  DCHECK(OwnsLayer());
  auto old_layer = std::move(owned_layer_);
  layer_->owner_ = nullptr;

  // Since, |new_layer| doesn't have content yet. We don't insert |new_layer|
  // into layer tree for not displaying it. We also don't move child layers.
  auto const new_layer = new Layer();
  new_layer->SetBounds(old_layer->bounds());
  SetLayer(new_layer);

  // Migrate child layers to |new_layer|. |delegate_| may recreate child layers
  while (auto const child_layer = old_layer->first_child())
    new_layer->AppendLayer(child_layer);

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
