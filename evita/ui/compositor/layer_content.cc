// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/compositor/layer_content.h"

#include <dcomp.h>

#include "evita/gfx/canvas.h"
#include "evita/gfx/rect_conversions.h"
#include "evita/ui/compositor/compositor.h"
#include "evita/ui/compositor/layer.h"

namespace ui {

namespace {
//////////////////////////////////////////////////////////////////////
//
// CanvasForSurface
//
class CanvasForSurface final : public gfx::Canvas {
 public:
  explicit CanvasForSurface(LayerContent* content);
  ~CanvasForSurface() = default;

 private:
  // gfx::Canvas
  void DidChangeBounds(const gfx::RectF& new_bounds) final;
  void DidLostRenderTarget() final;
  ID2D1RenderTarget* GetRenderTarget() const final;
  bool IsReady() final;

  common::ComPtr<ID2D1DeviceContext> d2d_device_context_;

  DISALLOW_COPY_AND_ASSIGN(CanvasForSurface);
};

CanvasForSurface::CanvasForSurface(LayerContent* content) {
  POINT offset;
  COM_VERIFY(content->surface()->BeginDraw(
      nullptr, IID_PPV_ARGS(&d2d_device_context_), &offset));
  auto const bounds = gfx::RectF(
      gfx::PointF(static_cast<float>(offset.x), static_cast<float>(offset.y)),
      content->layer()->bounds().size());
  SetInitialBounds(bounds);
}

// gfx::Canvas
void CanvasForSurface::DidChangeBounds(const gfx::RectF&) {
  NOTREACHED();
}
void CanvasForSurface::DidLostRenderTarget() {
  NOTREACHED();
}

ID2D1RenderTarget* CanvasForSurface::GetRenderTarget() const {
  return d2d_device_context_;
}

bool CanvasForSurface::IsReady() {
  return true;
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// LayerContent::DrawingScope
//
LayerContent::DrawingScope::DrawingScope(LayerContent* content)
    : content_(content) {
  auto const layer_bounds = content_->layer()->bounds();
  if (content_->bounds_ != layer_bounds) {
    content_->bounds_ = layer_bounds;
    content_->surface_.reset();
  }

  if (!content_->surface_) {
    auto const enclosing_bounds = gfx::ToEnclosingRect(layer_bounds);
    COM_VERIFY(Compositor::instance()->device()->CreateSurface(
        static_cast<UINT>(enclosing_bounds.width()),
        static_cast<UINT>(enclosing_bounds.height()),
        DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_ALPHA_MODE_PREMULTIPLIED,
        &content_->surface_));
    COM_VERIFY(content_->layer()->visual()->SetContent(content_->surface_));
  }

  canvas_.reset(new CanvasForSurface(content_));
}

LayerContent::DrawingScope::~DrawingScope() {
  COM_VERIFY(content_->surface_->EndDraw());
  Compositor::instance()->NeedCommit();
}

//////////////////////////////////////////////////////////////////////
//
// LayerContent
//
LayerContent::LayerContent(Layer* layer) : layer_(layer) {}

LayerContent::~LayerContent() {}

}  // namespace ui
