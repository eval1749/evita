// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_COMPOSITOR_LAYER_CONTENT_H_
#define EVITA_UI_COMPOSITOR_LAYER_CONTENT_H_

#include <memory>

struct IDCompositionSurface;

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
 public:
  class DrawingScope final {
   public:
    explicit DrawingScope(LayerContent* content);
    ~DrawingScope();

    gfx::Canvas* canvas() const { return canvas_.get(); }

   private:
    std::unique_ptr<gfx::Canvas> canvas_;
    LayerContent* content_;

    DISALLOW_COPY_AND_ASSIGN(DrawingScope);
  };

  explicit LayerContent(Layer* layer);
  ~LayerContent();

  Layer* layer() const { return layer_; }
  IDCompositionSurface* surface() const { return surface_; }

 private:
  friend class DrawingScope;

  gfx::RectF bounds_;
  Layer* layer_;
  common::ComPtr<IDCompositionSurface> surface_;

  DISALLOW_COPY_AND_ASSIGN(LayerContent);
};

}  // namespace ui

#endif  // EVITA_UI_COMPOSITOR_LAYER_CONTENT_H_
