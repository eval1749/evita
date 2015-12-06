// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_COMPOSITOR_COMPOSITOR_H_
#define EVITA_UI_COMPOSITOR_COMPOSITOR_H_

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
class Compositor final {
 public:
  explicit Compositor(gfx::DxDevice* device);
  ~Compositor();

  IDCompositionDesktopDevice* desktop_device() const { return desktop_device_; }
  gfx::DxDevice* device() const { return device_; }
  static Compositor* instance();

  void CommitIfNeeded();
  common::ComPtr<IDCompositionVisual2> CreateVisual();
  void NeedCommit() { need_commit_ = true; }
  void WaitForCommitCompletion();

 private:
  common::ComPtr<IDCompositionDesktopDevice> desktop_device_;
  gfx::DxDevice* const device_;
  bool need_commit_;

  DISALLOW_COPY_AND_ASSIGN(Compositor);
};

}  // namespace ui

#endif  // EVITA_UI_COMPOSITOR_COMPOSITOR_H_
