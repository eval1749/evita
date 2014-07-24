// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_compositor_compositor_h)
#define INCLUDE_evita_ui_compositor_compositor_h

#include <memory>

#include "base/basictypes.h"
#include "common/win/scoped_comptr.h"

interface IDCompositionDesktopDevice;
interface IDCompositionVisual2;
interface ID2D1Device;

namespace gfx {
class DxDevice;
}

namespace ui {

class Layer;

//////////////////////////////////////////////////////////////////////
//
// Compositor
//
class Compositor {
  private: gfx::DxDevice* dx_device_;
  private: common::ComPtr<IDCompositionDesktopDevice> composition_device_;
  private: bool need_commit_;
  private: std::unique_ptr<ui::Layer> layer_;

  public: Compositor(gfx::DxDevice* dx_device, HWND hwnd);
  public: ~Compositor();

  public: IDCompositionDesktopDevice* device() const {
    return composition_device_;
  }
  public: gfx::DxDevice* dx_device() const { return dx_device_; }
  public: Layer* layer() const { return layer_.get(); }

  public: void Commit();
  public: common::ComPtr<IDCompositionVisual2> CreateVisual();
  public: void NeedCommit() { need_commit_ = true; }

  DISALLOW_COPY_AND_ASSIGN(Compositor);
};

} // namespace ui

#endif //!defined(INCLUDE_evita_ui_compositor_compositor_h)
