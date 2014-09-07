// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_compositor_layer_owner_delegate_h)
#define INCLUDE_evita_ui_compositor_layer_owner_delegate_h

#include "base/macros.h"

namespace ui {

class Layer;

//////////////////////////////////////////////////////////////////////
//
// LayerOwnerDelegate
//
class LayerOwnerDelegate {
  protected: LayerOwnerDelegate();
  public: virtual ~LayerOwnerDelegate();

  public: virtual void DidRecreateLayer(Layer* old_layer) = 0;

  DISALLOW_COPY_AND_ASSIGN(LayerOwnerDelegate);
};

} // namespace ui

#endif //!defined(INCLUDE_evita_ui_compositor_layer_owner_delegate_h)
