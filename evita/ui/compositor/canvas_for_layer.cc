// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/compositor/canvas_for_layer.h"

#include <dcomp.h>
#include <dxgi1_3.h>

#include "base/logging.h"
#include "evita/gfx/dx_device.h"
#include "evita/gfx/swap_chain.h"
#include "evita/ui/compositor/compositor.h"
#include "evita/ui/compositor/layer.h"

namespace gfx {

//////////////////////////////////////////////////////////////////////
//
// CanvasForLayer
//
CanvasForLayer::CanvasForLayer(ui::Layer* layer)
    : layer_(layer),
      swap_chain_(SwapChain::CreateForComposition(layer->bounds())) {
  DCHECK_EQ(swap_chain_->bounds().size(), layer->bounds().size());
  SetInitialBounds(swap_chain_->bounds());
}

CanvasForLayer::~CanvasForLayer() {}

// Canvas
void CanvasForLayer::AddDirtyRectImpl(const RectF& new_dirty_rect) {
  swap_chain_->AddDirtyRect(new_dirty_rect);
}

void CanvasForLayer::DidCallEndDraw() {
  swap_chain_->Present();
}

void CanvasForLayer::DidChangeBounds(const RectF& new_bounds) {
  swap_chain_->DidChangeBounds(new_bounds);
}

void CanvasForLayer::DidLostRenderTarget() {
  DCHECK(!layer_->bounds().empty());
  layer_->visual()->SetContent(nullptr);
  swap_chain_.reset(SwapChain::CreateForComposition(layer_->bounds()));
  layer_->visual()->SetContent(swap_chain_->swap_chain());
  DidCreateRenderTarget();
}

ID2D1RenderTarget* CanvasForLayer::GetRenderTarget() const {
  return swap_chain_->d2d_device_context();
}

bool CanvasForLayer::IsReady() {
  return swap_chain_->IsReady();
}

}  // namespace gfx
