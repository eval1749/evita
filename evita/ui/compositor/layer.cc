// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/compositor/layer.h"

#include <dcomp.h>
#include <dxgi1_3.h>

#include "base/logging.h"
#include "evita/gfx/canvas.h"
#include "evita/gfx/color_f.h"
#include "evita/gfx/swap_chain.h"
#include "evita/ui/compositor/compositor.h"
#include "evita/ui/compositor/canvas_for_layer.h"

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// Layer
//
Layer::Layer(Compositor* compositor)
    : compositor_(compositor), visual_(compositor->CreateVisual()) {
  COM_VERIFY(visual_->SetBitmapInterpolationMode(
      DCOMPOSITION_BITMAP_INTERPOLATION_MODE_LINEAR));
  COM_VERIFY(visual_->SetBorderMode(DCOMPOSITION_BORDER_MODE_SOFT));

  common::ComPtr<IDCompositionVisualDebug> debug_visual;
  COM_VERIFY(debug_visual.QueryFrom(visual_));
  // TODO(eval1749) It seems heat map is painted with alpha=1.0.
  COM_VERIFY(debug_visual->EnableHeatMap(gfx::ColorF(255, 255, 0, 0.1)));
  //COM_VERIFY(debug_visual->EnableRedrawRegions());
}

Layer::~Layer() {
  visual_->SetContent(nullptr);
  visual_->RemoveAllVisuals();
}

void Layer::AppendChildLayer(Layer* new_child_layer) {
  child_layers_.push_back(new_child_layer);
  auto const is_insert_above = true;
  auto const ref_visual = static_cast<IDCompositionVisual*>(nullptr);
  COM_VERIFY(visual_->AddVisual(new_child_layer->visual_, is_insert_above,
                                ref_visual));
  compositor_->NeedCommit();
}

gfx::Canvas* Layer::CreateCanvas() {
  DCHECK(!bounds_.empty());
  auto canvas = new gfx::CanvasForLayer(this);
  COM_VERIFY(visual_->SetContent(canvas->swap_chain()->swap_chain()));
  return canvas;
}

void Layer::DidChangeBounds() {
  compositor_->NeedCommit();
}

void Layer::SetBounds(const gfx::RectF& new_bounds) {
  DCHECK(!new_bounds.empty());
  auto changed = false;
  if (bounds_.left != new_bounds.left) {
    COM_VERIFY(visual_->SetOffsetX(new_bounds.left));
    bounds_.set_origin(gfx::PointF(new_bounds.left, bounds_.top));
    changed = true;
  }
  if (bounds_.top != new_bounds.top) {
    COM_VERIFY(visual_->SetOffsetY(new_bounds.top));
    bounds_.set_origin(gfx::PointF(bounds_.left, new_bounds.top));
    changed = true;
  }

  if (bounds_.size() != new_bounds.size()) {
    bounds_.set_size(new_bounds.size());
    changed = true;
  }

  if (!changed)
    return;

  DidChangeBounds();
}

//////////////////////////////////////////////////////////////////////
//
// HwndLayer
//
HwndLayer::HwndLayer(Compositor* compositor, HWND hwnd)
    : Layer(compositor) {
  ::SetLastError(0);
  ::SetWindowLong(hwnd, GWL_EXSTYLE,
                  ::GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
  if (auto const last_error = ::GetLastError()) {
    DVLOG(ERROR) << "SetWindowLong GWL_EXSTYLE err=" << last_error;
    NOTREACHED();
  }
  DVLOG(0) << "ex_style=0x" << std::hex << ::GetWindowLong(hwnd, GWL_EXSTYLE);
  //DCHECK(::GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_LAYERED);
  common::ComPtr<IUnknown> surface;
  COM_VERIFY(compositor->device()->CreateSurfaceFromHwnd(hwnd, &surface));
  visual()->SetContent(surface);
  RECT bounds;
  ::GetClientRect(hwnd, &bounds);
  visual()->SetOffsetX(static_cast<float>(bounds.left));
  visual()->SetOffsetY(static_cast<float>(bounds.top));
}

HwndLayer::~HwndLayer() {
}

}  // namespace ui
