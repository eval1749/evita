// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/animation/animator.h"

#include "base/logging.h"
#include "base/time/time.h"
#include "evita/ui/animation/animatable.h"
#include "evita/ui/animation/animation_observer.h"

namespace ui {

Animator::Animator() {}

Animator::~Animator() {}

base::Time Animator::current_time() const {
  DCHECK(is_playing());
  return current_time_;
}

bool Animator::is_playing() const {
  return current_time_ != base::Time();
}

void Animator::Animate(Animatable* animatable) {
  DCHECK_EQ(this, animatable->animator_);
  running_animatables_.insert(animatable);
  if (animatable->is_finished_)
    return;
  animatable->Animate(current_time_);
}

void Animator::CancelAnimation(Animatable* animatable) {
  DCHECK_EQ(this, animatable->animator_);
  DCHECK(!animatable->is_finished_);
  animatable->animator_ = nullptr;
  pending_animatables_.erase(animatable);
  running_animatables_.erase(animatable);
  waiting_animatables_.erase(animatable);
  animatable->DidCancelAnimation();
}

void Animator::EndAnimate() {
  DCHECK(is_playing());
  for (auto const animatable : running_animatables_) {
    if (animatable->is_finished_) {
      animatable->animator_ = nullptr;
      animatable->DidFinishAnimation();
      delete animatable;
      continue;
    }
    if (waiting_animatables_.find(animatable) == waiting_animatables_.end())
      animatable->animator_ = nullptr;
  }
  running_animatables_.clear();
  current_time_ = base::Time();
}

void Animator::PlayAnimations(base::Time now) {
  DCHECK(!is_playing());
  DCHECK(running_animatables_.empty());
  if (waiting_animatables_.empty())
    return;
  pending_animatables_.swap(waiting_animatables_);
  StartAnimate(now);
  while (!pending_animatables_.empty()) {
    auto const present = pending_animatables_.begin();
    auto const animatable = *present;
    pending_animatables_.erase(present);
    Animate(animatable);
  }
  EndAnimate();
}

void Animator::ScheduleAnimation(Animatable* animatable) {
  DCHECK(!animatable->is_finished_);
  DCHECK(!animatable->animator_ || animatable->animator_ == this);
  animatable->animator_ = this;
  waiting_animatables_.insert(animatable);
  RequestAnimationFrame();
}

void Animator::StartAnimate(base::Time now) {
  DCHECK(!is_playing());
  current_time_ = now;
}

// AnimationFrameHandler
void Animator::DidBeginAnimationFrame(base::Time time) {
  PlayAnimations(time);
}

}  // namespace ui
