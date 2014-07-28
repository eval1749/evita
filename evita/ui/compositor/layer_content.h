// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_compositor_layer_content_h)
#define INCLUDE_evita_ui_compositor_layer_content_h

#include <memory>

interface IDCompositionSurface;

#include "common/win/scoped_comptr.h"
#include "evita/gfx/rect.h"
#include "evita/gfx/rect_f.h"

namespace gfx {
class Canvas;
}

namespace ui {

class Layer;

//////////////////////////////////////////////////////////////////////
//
// LayerCotnent
// A wrapper of direct composition surface.
//
class LayerContent final {
  public: class DrawingScope {
    private: std::unique_ptr<gfx::Canvas> canvas_;
    private: LayerContent* content_;

    public: DrawingScope(LayerContent* content);
    public: ~DrawingScope();

    public: gfx::Canvas* canvas() const { return canvas_.get(); }

    DISALLOW_COPY_AND_ASSIGN(DrawingScope);
  };
  friend class DrawingScope;

  private: gfx::RectF bounds_;
  private: Layer* layer_;
  private: common::ComPtr<IDCompositionSurface> surface_;

  public: LayerContent(Layer* layer);
  public: ~LayerContent();

  public: Layer* layer() const { return layer_; }
  public: IDCompositionSurface* surface() const { return surface_; }

  DISALLOW_COPY_AND_ASSIGN(LayerContent);
};

} // namespace ui

#endif //!defined(INCLUDE_evita_ui_compositor_layer_content_h)
