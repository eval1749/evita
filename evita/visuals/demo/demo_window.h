// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DEMO_DEMO_WINDOW_H_
#define EVITA_VISUALS_DEMO_DEMO_WINDOW_H_

#include <memory>

#include "base/callback.h"
#include "evita/ui/animation/animatable_window.h"
#include "evita/ui/compositor/layer_owner_delegate.h"

namespace gfx {
class Canvas;
}

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// DemoWindow
//
class DemoWindow final : public ui::AnimatableWindow,
                         public ui::LayerOwnerDelegate {
 public:
  explicit DemoWindow(const base::Closure& quit_closure);
  ~DemoWindow() final;

  gfx::Canvas* GetCanvas() const;

 private:
  // ui::AnimationFrameHandler
  void DidBeginAnimationFrame(base::Time time) final;

  // ui::LayerOwnerDelegate
  void DidRecreateLayer(ui::Layer* old_layer) final;

  // ui::Widget
  void CreateNativeWindow() const final;
  void DidChangeBounds() final;
  void DidHide() final;
  void DidRealize() final;
  void DidShow() final;

  base::Closure quit_closure_;

  std::unique_ptr<gfx::Canvas> canvas_;

  DISALLOW_COPY_AND_ASSIGN(DemoWindow);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_DEMO_DEMO_WINDOW_H_
