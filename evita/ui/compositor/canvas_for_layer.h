// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_compositor_canvas_for_layer_h)
#define INCLUDE_evita_ui_compositor_canvas_for_layer_h

#include "evita/gfx/canvas.h"

namespace ui {
class Layer;
}

namespace gfx {

//////////////////////////////////////////////////////////////////////
//
// CanvasForLayer
//
class CanvasForLayer : public Canvas {
  private: ui::Layer* layer_;
  private: std::unique_ptr<SwapChain> swap_chain_;

  public: CanvasForLayer(ui::Layer* layer);
  public: virtual ~CanvasForLayer();

  public: SwapChain* swap_chain() const { return swap_chain_.get(); }

  // Canvas
  private: virtual void AddDirtyRect(const RectF& new_dirty_rect) override;
  private: virtual void DidCallEndDraw() override;
  private: virtual void DidChangeBounds(const RectF& new_bounds) override;
  private: virtual void DidLostRenderTarget() override;
  private: virtual ID2D1RenderTarget* GetRenderTarget() const override;

  DISALLOW_COPY_AND_ASSIGN(CanvasForLayer);
};

} // namespace gfx

#endif //!defined(INCLUDE_evita_ui_compositor_canvas_for_layer_h)
