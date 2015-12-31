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

class FloatPoint;
class FloatRect;

//////////////////////////////////////////////////////////////////////
//
// WindowEventHandler
//
class WindowEventHandler {
 public:
  virtual ~WindowEventHandler();

  virtual void DidChangeWindowBounds(const FloatRect& bounds) = 0;
  virtual void DidMoveMouse(const FloatPoint& point) = 0;

 protected:
  WindowEventHandler();

 private:
  DISALLOW_COPY_AND_ASSIGN(WindowEventHandler);
};

//////////////////////////////////////////////////////////////////////
//
// DemoWindow
//
class DemoWindow final : public ui::AnimatableWindow,
                         public ui::LayerOwnerDelegate {
 public:
  DemoWindow(WindowEventHandler* event_handler,
             const base::Closure& quit_closure);
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
  void OnMouseMoved(const ui::MouseEvent& event) final;

  std::unique_ptr<gfx::Canvas> canvas_;
  WindowEventHandler* const event_handler_;
  base::Closure quit_closure_;

  DISALLOW_COPY_AND_ASSIGN(DemoWindow);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_DEMO_DEMO_WINDOW_H_
