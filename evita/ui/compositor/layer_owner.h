// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_COMPOSITOR_LAYER_OWNER_H_
#define EVITA_UI_COMPOSITOR_LAYER_OWNER_H_

#include <memory>

#include "base/macros.h"

namespace ui {

class Layer;
class LayerOwnerDelegate;

//////////////////////////////////////////////////////////////////////
//
// LayerOwner
//
class LayerOwner {
 public:
  virtual ~LayerOwner();

  Layer* layer() { return layer_; }
  const Layer* layer() const { return layer_; }
  void set_layer_owner_delegate(LayerOwnerDelegate* delegate);

  std::unique_ptr<Layer> AcquireLayerTree();
  std::unique_ptr<Layer> RecreateLayer();
  void SetLayer(Layer* layer);

 protected:
  LayerOwner();

  void DestroyLayer();
  bool OwnsLayer() const;

 private:
  Layer* DetachLayerTree();

  std::unique_ptr<Layer> owned_layer_;
  Layer* layer_;
  LayerOwnerDelegate* delegate_;

  DISALLOW_COPY_AND_ASSIGN(LayerOwner);
};

}  // namespace ui

#endif  // EVITA_UI_COMPOSITOR_LAYER_OWNER_H_
