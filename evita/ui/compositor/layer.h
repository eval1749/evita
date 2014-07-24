// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_compositor_layer_h)
#define INCLUDE_evita_ui_compositor_layer_h

#include <vector>

#include "base/basictypes.h"
#include "common/win/scoped_comptr.h"
#include "evita/gfx/rect_f.h"

interface IDCompositionVisual2;

namespace gfx {
class Canvas;
}

namespace ui {

class Compositor;

class Layer  {
  private: gfx::RectF bounds_;
  private: std::vector<Layer*> child_layers_;
  private: Compositor* compositor_;
  private: common::ComPtr<IDCompositionVisual2> visual_;

  public: Layer(Compositor* compositor);
  public: virtual ~Layer();

  public: Compositor* compositor() const { return compositor_; }
  public: operator IDCompositionVisual2*() const { return visual_; }

  public: const gfx::RectF& bounds() const { return bounds_; }
  public: IDCompositionVisual2* visual() const { return visual_; }

  public: void AppendChildLayer(Layer* new_child);
  public: gfx::Canvas* CreateCanvas();
  protected: virtual void DidChangeBounds();
  public: void SetBounds(const gfx::RectF& new_bounds);

  DISALLOW_COPY_AND_ASSIGN(Layer);
};

class HwndLayer : public Layer {
  public: HwndLayer(Compositor* compositor, HWND hwnd);
  public: virtual ~HwndLayer();

  DISALLOW_COPY_AND_ASSIGN(HwndLayer);
};

} // namespace ui

#endif //!defined(INCLUDE_evita_ui_compositor_layer_h)
