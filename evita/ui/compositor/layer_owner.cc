// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <utility>

#include "evita/ui/compositor/layer_owner.h"

#include "base/logging.h"
#include "evita/ui/compositor/layer.h"

namespace ui {

LayerOwner::LayerOwner() : layer_(nullptr) {
}

LayerOwner::~LayerOwner() {
}

std::unique_ptr<Layer> LayerOwner::AcquireLayer() {
  if (owned_layer_)
    owned_layer_->owner_ = nullptr;
  return std::move(owned_layer_);
}

void LayerOwner::DestroyLayer() {
  layer_ = nullptr;
  owned_layer_.reset();
}

bool LayerOwner::OwnsLayer() const {
  return !!owned_layer_;
}

void LayerOwner::SetLayer(Layer* layer) {
  DCHECK(!OwnsLayer());
  owned_layer_.reset(layer);
  layer_ = layer;
  layer_->owner_ = this;
}

}  // namespace ui
