// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/animation/animation_scheduler.h"

#include "base/time/time.h"
#include "evita/ui/animation/animation_frame_handler.h"

namespace ui {

AnimationScheduler::AnimationScheduler() {
}

AnimationScheduler::~AnimationScheduler() {
}

void AnimationScheduler::CancelAnimationFrameRequest(
    AnimationFrameHandler* handler) {
  pending_handlers_.erase(handler);
  canceled_handlers_.insert(handler);
}

void AnimationScheduler::HandleAnimationFrame(base::Time time) {
  auto running_handlers = pending_handlers_;
  pending_handlers_.clear();
  for (auto handler : running_handlers) {
    if (canceled_handlers_.find(handler) != canceled_handlers_.end())
      continue;
    handler->HandleAnimationFrame(time);
  }
  canceled_handlers_.clear();
}

void AnimationScheduler::RequestAnimationFrame(
    AnimationFrameHandler* handler) {
  pending_handlers_.insert(handler);
}

}  // namespace ui
