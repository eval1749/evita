// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/animation/animatable_window.h"

#include "evita/ui/animation/animator.h"

namespace ui {

AnimatableWindow::AnimatableWindow(std::unique_ptr<NativeWindow> native_window)
    : Widget(std::move(native_window)) {}

AnimatableWindow::AnimatableWindow() {}

AnimatableWindow::~AnimatableWindow() {}

// AnimationFrameHandler
const char* AnimatableWindow::GetAnimationFrameType() const {
  return class_name();
}

// Widget
void AnimatableWindow::DidChangeBounds() {
  Widget::DidChangeBounds();
  RequestAnimationFrame();
}

void AnimatableWindow::DidRealize() {
  Widget::DidRealize();
  RequestAnimationFrame();
}

void AnimatableWindow::DidShow() {
  Widget::DidShow();
  RequestAnimationFrame();
}

void AnimatableWindow::WillDestroyWidget() {
  Widget::WillDestroyWidget();
  CancelAnimationFrameRequest();
}

}  // namespace ui
