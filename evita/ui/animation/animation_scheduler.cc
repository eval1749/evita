// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/animation/animation_scheduler.h"

#include "base/memory/singleton.h"

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// AnimationScheduler
//
AnimationScheduler::AnimationScheduler() {}
AnimationScheduler::~AnimationScheduler() {}

void AnimationScheduler::CancelAnimationFrameRequest(
    AnimationFrameHandler* handler) {
  if (!impl_)
    return;
  impl_->CancelAnimationFrameRequest(handler);
}

// Request animation frame.
void AnimationScheduler::RequestAnimationFrame(AnimationFrameHandler* handler) {
  if (!impl_)
    return;
  impl_->RequestAnimationFrame(handler);
}

void AnimationScheduler::SetScheduler(AnimationScheduler* scheduler) {
  impl_ = scheduler;
}

// static
AnimationScheduler* AnimationScheduler::GetInstance() {
  return base::Singleton<AnimationScheduler>::get();
}

}  // namespace ui
