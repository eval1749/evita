// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/animation/animator.h"

#include <vector>

#include "base/logging.h"
#include "base/time/time.h"
#include "evita/ui/animation/animatable.h"
#include "evita/ui/animation/animation_observer.h"

namespace ui {

Animator::Animator() {
}

Animator::~Animator() {
}

base::Time Animator::current_time() const {
  DCHECK(is_playing());
  return current_time_;
}

bool Animator::is_playing() const {
  return current_time_ != base::Time();
}

void Animator::Animate(Animatable* animatable) {
  DCHECK_EQ(Animatable::State::NotScheduled, animatable->state_);
  animatable->animator_ = this;
  animatable->state_ = Animatable::State::Playing;
  animatable->Animate(current_time_);
  switch (animatable->state_) {
    case Animatable::State::Finished:
      DCHECK_EQ(this, animatable->animator_);
      FOR_EACH_OBSERVER(AnimationObserver, animatable->observers_,
                        DidFinishAnimation(animatable));
      will_schedule_animatables_.erase(animatable);
      will_delete_animatables_.insert(animatable);
      return;
    case Animatable::State::NotScheduled:
      DCHECK(!animatable->animator_);
      NOTREACHED();
      return;
    case Animatable::State::Playing:
      DCHECK_EQ(this, animatable->animator_);
      animatable->animator_ = nullptr;
      animatable->state_ = Animatable::State::NotScheduled;
      return;
    case Animatable::State::Scheduled:
      // |animatable| can be scheduled in another animator.
      DCHECK(animatable->animator_);
      return;
    default:
      NOTREACHED();
      return;
  }
}

void Animator::CancelAnimation(Animatable* animatable) {
  switch (animatable->state_) {
    case Animatable::State::NotScheduled:
      DCHECK(!animatable->animator_);
      if (is_playing())
        will_schedule_animatables_.erase(animatable);
      return;
    case Animatable::State::Finished:
    case Animatable::State::Playing:
      NOTREACHED();
      return;
    case Animatable::State::Scheduled: {
      DCHECK_EQ(this, animatable->animator_);
      auto const present = animatables_.find(animatable);
      DCHECK(present != animatables_.end());
      animatable->animator_ = nullptr;
      animatable->state_ = Animatable::State::NotScheduled;
      animatables_.erase(present);
      return;
    }
    default:
      NOTREACHED();
      return;
  }
}

void Animator::EndAnimate() {
  DCHECK(is_playing());
  current_time_ = base::Time();
  while (!will_schedule_animatables_.empty()) {
    auto const present = will_schedule_animatables_.begin();
    auto const animatable = *present;
    DCHECK(!animatable->animator_);
    DCHECK_EQ(Animatable::State::NotScheduled, animatable->state_);
    will_schedule_animatables_.erase(present);
    ScheduleAnimation(animatable);
  }
  while (!will_delete_animatables_.empty()) {
    auto const present = will_delete_animatables_.begin();
    auto const animatable = *present;
    DCHECK_EQ(this, animatable->animator_);
    DCHECK_EQ(Animatable::State::Finished, animatable->state_);
    will_delete_animatables_.erase(present);
    animatable->animator_ = nullptr;
    animatable->state_ = Animatable::State::NotScheduled;
    delete animatable;
  }
}

void Animator::PlayAnimation(base::Time now, Animatable* animatable) {
  DCHECK(!is_playing());
  ui::Animator::instance()->CancelAnimation(animatable);
  StartAnimate(now);
  Animate(animatable);
  EndAnimate();
}

void Animator::PlayAnimations(base::Time now) {
  DCHECK(!is_playing());
  std::vector<Animatable*> animatables(animatables_.size());
  animatables.resize(0);
  for (auto animatable : animatables_) {
    DCHECK_EQ(Animatable::State::Scheduled, animatable->state_);
    animatable->animator_ = nullptr;
    animatable->state_ = Animatable::State::NotScheduled;
    animatables.push_back(animatable);
  }
  animatables_.clear();
  StartAnimate(now);
  for (auto animatable : animatables) {
    Animate(animatable);
  }
  EndAnimate();
}

void Animator::ScheduleAnimation(Animatable* animatable) {
  if (is_playing()) {
    DCHECK_NE(Animatable::State::Finished, animatable->state_);
    will_schedule_animatables_.insert(animatable);
    return;
  }
  switch (animatable->state_) {
    case Animatable::State::NotScheduled:
      DCHECK(!animatable->animator_);
      animatable->animator_ = this;
      animatable->state_ = Animatable::State::Scheduled;
      animatables_.insert(animatable);
      return;
    case Animatable::State::Finished:
      NOTREACHED() << this << " is already finished.";
      return;
    case Animatable::State::Playing:
      animatable->animator_ = this;
      animatable->state_ = Animatable::State::Scheduled;
      animatables_.insert(animatable);
      return;
    case Animatable::State::Scheduled:
      DCHECK_EQ(this, animatable->animator_);
      return;
    default:
      NOTREACHED();
      return;
  }
}

void Animator::StartAnimate(base::Time now) {
  DCHECK(!is_playing());
  DCHECK(will_delete_animatables_.empty());
  DCHECK(will_schedule_animatables_.empty());
  current_time_ = now;
}

}  // namespace ui
