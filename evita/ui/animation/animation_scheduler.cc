// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/animation/animation_scheduler.h"

#include "base/synchronization/lock.h"
#include "base/time/time.h"
#include "evita/ui/animation/animation_frame_handler.h"

namespace ui {

AnimationScheduler::AnimationScheduler() : lock_(new base::Lock()) {
}

AnimationScheduler::~AnimationScheduler() {
}

void AnimationScheduler::CancelAnimationFrameRequest(
    AnimationFrameHandler* handler) {
  base::AutoLock lock_scope(*lock_);
  pending_handlers_.erase(handler);
  canceled_handlers_.insert(handler);
}

void AnimationScheduler::HandleAnimationFrame(base::Time time) {
  std::unordered_set<AnimationFrameHandler*> running_handlers;
  std::unordered_set<AnimationFrameHandler*> canceling_handlers;
  {
    base::AutoLock lock_scope(*lock_);
    running_handlers.swap(pending_handlers_);
    canceling_handlers.swap(canceled_handlers_);
  }
  for (auto handler : running_handlers) {
    if (canceling_handlers.find(handler) != canceling_handlers.end())
      continue;
    handler->HandleAnimationFrame(time);
  }
}

void AnimationScheduler::RequestAnimationFrame(
    AnimationFrameHandler* handler) {
  base::AutoLock lock_scope(*lock_);
  pending_handlers_.insert(handler);
}

}  // namespace ui
