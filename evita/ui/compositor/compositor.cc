// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/compositor/compositor.h"

// L1 C4917 'declarator' : a GUID can only be associated with a class, interface
// or namespace
#pragma warning(disable: 4917)
#include <dcomp.h>

#include "evita/gfx/dx_device.h"
#include "evita/ui/compositor/layer.h"

#pragma comment(lib, "dcomp.lib")

namespace ui {

Compositor::Compositor(gfx::DxDevice* dx_device, HWND hwnd)
    : dx_device_(dx_device), need_commit_(false) {
  DCHECK(hwnd);
  COM_VERIFY(::DCompositionCreateDevice2(
      dx_device->d2d_device(),
      IID_PPV_ARGS(&composition_device_)));

  common::ComPtr<IDCompositionDeviceDebug> debug_device;
  COM_VERIFY(debug_device.QueryFrom(composition_device_));
  debug_device->EnableDebugCounters();

  layer_.reset(new Layer(this));

#if 0
  common::ComPtr<IDCompositionTarget> composition_target;
  COM_VERIFY(composition_device_->CreateTargetForHwnd(hwnd, false,
                                                      &composition_target));
  composition_target->SetRoot(layer_->visual());
#endif
  {
    RECT bounds;
    ::GetClientRect(hwnd, &bounds);
    layer_->SetBounds(gfx::RectF(gfx::PointF(), gfx::SizeF(
        static_cast<float>(bounds.right - bounds.left),
        static_cast<float>(bounds.bottom - bounds.top))));
  }
}

Compositor::~Compositor() {
}

void Compositor::Commit() {
  if (!need_commit_)
    return;
#if _DEBUG
  common::ComPtr<IDCompositionDevice> device;
  BOOL is_valid;
  COM_VERIFY(device.QueryFrom(composition_device_));
  COM_VERIFY(device->CheckDeviceState(&is_valid));
  DCHECK(is_valid);
#endif
  COM_VERIFY(composition_device_->Commit());
  need_commit_ = false;
}

common::ComPtr<IDCompositionVisual2> Compositor::CreateVisual() {
  common::ComPtr<IDCompositionVisual2> visual;
  COM_VERIFY(composition_device_->CreateVisual(&visual));
  return visual;
}

}  // namespace ui
