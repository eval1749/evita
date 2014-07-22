// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_compositor_compositor_h)
#define INCLUDE_evita_ui_compositor_compositor_h

#include "base/basictypes.h"
#include "common/win/scoped_comptr.h"

interface IDCompositionDesktopDevice;
interface IDCompositionVisual2;
interface ID2D1Device;

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// Compositor
//
class Compositor {
  private: common::ComPtr<IDCompositionDesktopDevice> composition_device_;
  private: bool need_commit_;

  public: Compositor(ID2D1Device* d2d_device);
  public: ~Compositor();

  public: IDCompositionDesktopDevice* device() const {
    return composition_device_;
  }

  public: void Commit();
  public: common::ComPtr<IDCompositionVisual2> CreateVisual();
  public: void NeedCommit() { need_commit_ = true; }

  DISALLOW_COPY_AND_ASSIGN(Compositor);
};

} // namespace ui

#endif //!defined(INCLUDE_evita_ui_compositor_compositor_h)
