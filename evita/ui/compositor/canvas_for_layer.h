// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_COMPOSITOR_CANVAS_FOR_LAYER_H_
#define EVITA_UI_COMPOSITOR_CANVAS_FOR_LAYER_H_

#include "evita/gfx/canvas.h"

namespace ui {
class Layer;
}

namespace gfx {

//////////////////////////////////////////////////////////////////////
//
// CanvasForLayer
//
class CanvasForLayer final : public Canvas {
 public:
  explicit CanvasForLayer(ui::Layer* layer);
  ~CanvasForLayer() final;

  SwapChain* swap_chain() const { return swap_chain_.get(); }

 private:
  // Canvas
  void AddDirtyRectImpl(const RectF& new_dirty_rect) final;
  void DidCallEndDraw() final;
  void DidChangeBounds(const RectF& new_bounds) final;
  void DidLostRenderTarget() final;
  ID2D1RenderTarget* GetRenderTarget() const final;

  ui::Layer* layer_;
  std::unique_ptr<SwapChain> swap_chain_;

  DISALLOW_COPY_AND_ASSIGN(CanvasForLayer);
};

}  // namespace gfx

#endif  // EVITA_UI_COMPOSITOR_CANVAS_FOR_LAYER_H_
