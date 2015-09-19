// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_COMPOSITOR_LAYER_OWNER_DELEGATE_H_
#define EVITA_UI_COMPOSITOR_LAYER_OWNER_DELEGATE_H_

#include "base/macros.h"

namespace ui {

class Layer;

//////////////////////////////////////////////////////////////////////
//
// LayerOwnerDelegate
//
class LayerOwnerDelegate {
 public:
  virtual ~LayerOwnerDelegate();

  virtual void DidRecreateLayer(Layer* old_layer) = 0;

 protected:
  LayerOwnerDelegate();

 private:
  DISALLOW_COPY_AND_ASSIGN(LayerOwnerDelegate);
};

}  // namespace ui

#endif  // EVITA_UI_COMPOSITOR_LAYER_OWNER_DELEGATE_H_
