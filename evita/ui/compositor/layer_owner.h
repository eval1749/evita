// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_compositor_layer_owner_h)
#define INCLUDE_evita_ui_compositor_layer_owner_h

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
  private: std::unique_ptr<Layer> owned_layer_;
  private: Layer* layer_;
  private: LayerOwnerDelegate* delegate_;

  protected: LayerOwner();
  public: virtual ~LayerOwner();

  public: Layer* layer() { return layer_; }
  public: const Layer* layer() const { return layer_; }
  public: void set_layer_owner_delegate(LayerOwnerDelegate* delgate);

  public: std::unique_ptr<Layer> AcquireLayer();
  protected: void DestroyLayer();
  protected: bool OwnsLayer() const;
  public: std::unique_ptr<Layer> RecreateLayer();
  public: void SetLayer(Layer* layer);

  DISALLOW_COPY_AND_ASSIGN(LayerOwner);
};

} // namespace ui

#endif //!defined(INCLUDE_evita_ui_compositor_layer_owner_h)
