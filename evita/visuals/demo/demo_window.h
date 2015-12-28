// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DEMO_DEMO_WINDOW_H_
#define EVITA_VISUALS_DEMO_DEMO_WINDOW_H_

#include "base/callback.h"
#include "evita/ui/animation/animatable_window.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// DemoWindow
//
class DemoWindow final : public ui::AnimatableWindow {
 public:
  explicit DemoWindow(const base::Closure& quit_closure);
  ~DemoWindow() final;

 private:
  // ui::AnimationFrameHandler
  void DidBeginAnimationFrame(base::Time time) final;

  // ui::Widget
  void CreateNativeWindow() const final;

  base::Closure quit_closure_;

  DISALLOW_COPY_AND_ASSIGN(DemoWindow);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_DEMO_DEMO_WINDOW_H_
