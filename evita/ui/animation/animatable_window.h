// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_ANIMATION_ANIMATABLE_WINDOW_H_
#define EVITA_UI_ANIMATION_ANIMATABLE_WINDOW_H_

#include "evita/ui/animation/animation_frame_handler.h"
#include "evita/ui/widget.h"

namespace ui {

class AnimatableWindow : public AnimationFrameHandler, public Widget {
  DECLARE_CASTABLE_CLASS(AnimatableWindow, Widget);

 protected:
  explicit AnimatableWindow(std::unique_ptr<NativeWindow> native_window);
  AnimatableWindow();
  ~AnimatableWindow() override;

  // Widget
  void DidChangeBounds() override;
  void DidRealize() override;
  void DidShow() override;
  void WillDestroyWidget() override;

 private:
  DISALLOW_COPY_AND_ASSIGN(AnimatableWindow);
};

}  // namespace ui

#endif  // EVITA_UI_ANIMATION_ANIMATABLE_WINDOW_H_
