// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/animation/animatable_window.h"

#include "evita/ui/animation/animator.h"

namespace ui {

AnimatableWindow::AnimatableWindow(
    std::unique_ptr<NativeWindow>&& native_window)
    : Widget(std::move(native_window)) {
}

AnimatableWindow::AnimatableWindow() {
}

AnimatableWindow::~AnimatableWindow() {
}

// ui::Widget
void AnimatableWindow::DidHide() {
  ui::Widget::DidHide();
  ui::Animator::instance()->CancelAnimation(this);
}

void AnimatableWindow::DidRealize() {
  ui::Animator::instance()->ScheduleAnimation(this);
  Widget::DidRealize();
}

void AnimatableWindow::DidShow() {
  ui::Widget::DidShow();
  ui::Animator::instance()->ScheduleAnimation(this);
}
void AnimatableWindow::WillDestroyWidget() {
  Widget::WillDestroyWidget();
  ui::Animator::instance()->CancelAnimation(this);
}

}  // namespace ui
