// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_compositor_compositor_h)
#define INCLUDE_evita_ui_compositor_compositor_h

#include <memory>

#include "base/basictypes.h"
#include "common/memory/singleton.h"
#include "common/win/scoped_comptr.h"

interface IDCompositionDesktopDevice;
interface IDCompositionVisual2;
interface ID2D1Device;


namespace ui {

class Layer;

//////////////////////////////////////////////////////////////////////
//
// Compositor
//
class Compositor : public common::Singleton<Compositor> {
  DECLARE_SINGLETON_CLASS(Compositor);

  private: common::ComPtr<IDCompositionDesktopDevice> composition_device_;
  private: bool need_commit_;

  private: Compositor();
  // TODO(eval1749) We should destruct Compositor by Singleton destructor.
  public: virtual ~Compositor();

  public: IDCompositionDesktopDevice* device() const {
    return composition_device_;
  }
  public: void CommitIfNeeded();
  public: common::ComPtr<IDCompositionVisual2> CreateVisual();
  public: void NeedCommit() { need_commit_ = true; }

  DISALLOW_COPY_AND_ASSIGN(Compositor);
};

} // namespace ui

#endif //!defined(INCLUDE_evita_ui_compositor_compositor_h)
