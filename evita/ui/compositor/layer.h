// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_compositor_layer_h)
#define INCLUDE_evita_ui_compositor_layer_h

#include <unordered_set>

#include "base/basictypes.h"
#include "common/win/scoped_comptr.h"
#include "evita/gfx/rect.h"
#include "evita/gfx/rect_f.h"

interface IDCompositionVisual2;

namespace gfx {
class Canvas;
}

namespace ui {

class Layer  {
  private: gfx::RectF bounds_;
  private: std::unordered_set<Layer*> child_layers_;
  private: Layer* parent_layer_;
  private: Layer* top_most_layer_;
  private: common::ComPtr<IDCompositionVisual2> visual_;

  public: Layer();
  public: virtual ~Layer();

  public: operator IDCompositionVisual2*() const { return visual_; }

  public: const gfx::RectF& bounds() const { return bounds_; }
  public: IDCompositionVisual2* visual() const { return visual_; }

  public: void AppendChildLayer(Layer* new_child);
  public: gfx::Canvas* CreateCanvas();
  protected: virtual void DidChangeBounds();
  public: void RemoveChildLayer(Layer* old_child);
  public: void SetBounds(const gfx::RectF& new_bounds);
  public: void SetBounds(const gfx::Rect& new_bounds);
  public: void SetTopMostLayer(Layer* child);

  DISALLOW_COPY_AND_ASSIGN(Layer);
};

class HwndLayer : public Layer {
  public: HwndLayer(HWND hwnd);
  public: virtual ~HwndLayer();

  DISALLOW_COPY_AND_ASSIGN(HwndLayer);
};

} // namespace ui

#endif //!defined(INCLUDE_evita_ui_compositor_layer_h)
