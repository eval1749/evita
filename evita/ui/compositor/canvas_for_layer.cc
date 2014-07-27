// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/compositor/canvas_for_layer.h"

#include <dcomp.h>
#include <dxgi1_3.h>

#include "base/logging.h"
#include "evita/gfx/swap_chain.h"
#include "evita/ui/compositor/compositor.h"
#include "evita/ui/compositor/layer.h"

namespace gfx {

//////////////////////////////////////////////////////////////////////
//
// CanvasForLayer
//
CanvasForLayer::CanvasForLayer(ui::Layer* layer) : layer_(layer) {
  DCHECK(!layer_->bounds().empty());
  swap_chain_.reset(SwapChain::Create(
      layer->compositor()->dx_device(),
      SizeU(static_cast<uint32_t>(layer->bounds().width()),
            static_cast<uint32_t>(layer->bounds().height()))));
  DidCreateRenderTarget();
}

CanvasForLayer::~CanvasForLayer() {
}

// Canvas
void CanvasForLayer::AddDirtyRect(const Rect& new_dirty_rect) {
  swap_chain_->AddDirtyRect(new_dirty_rect);
}

void CanvasForLayer::DidCallEndDraw() {
  swap_chain_->Present();
}

void CanvasForLayer::DidChangeBounds(const SizeU& size) {
  swap_chain_->DidChangeBounds(size);
}

void CanvasForLayer::DidLostRenderTarget() {
  DCHECK(!layer_->bounds().empty());
  layer_->visual()->SetContent(nullptr);
  swap_chain_.reset(SwapChain::Create(
      layer_->compositor()->dx_device(),
      SizeU(static_cast<uint32_t>(layer_->bounds().width()),
            static_cast<uint32_t>(layer_->bounds().height()))));
  layer_->visual()->SetContent(swap_chain_->swap_chain());
  DidCreateRenderTarget();
}

ID2D1RenderTarget* CanvasForLayer::GetRenderTarget() const {
  return swap_chain_->d2d_device_context();
}

}  // namespace gfx
