// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/animation/animatable.h"

#include "evita/ui/animation/animation_observer.h"
#include "evita/ui/animation/animator.h"

namespace ui {

Animatable::Animatable() : animator_(nullptr), is_finished_(false) {
}

Animatable::~Animatable() {
  DCHECK(!animator_);
}

void Animatable::AddObserver(AnimationObserver* observer) {
  observers_.AddObserver(observer);
}

void Animatable::CancelAnimation() {
  if (!animator_ || is_finished_)
    return;
  animator_->CancelAnimation(this);
}

void Animatable::DidCancelAnimation() {
  DCHECK(!animator_);
  FOR_EACH_OBSERVER(AnimationObserver, observers_, DidCancelAnimation(this));
}

void Animatable::DidAnimate() {
  DCHECK(animator_);
  FOR_EACH_OBSERVER(AnimationObserver, observers_, DidAnimate(this));
}

void Animatable::DidFinishAnimation() {
  DCHECK(!animator_);
  DCHECK(is_finished_);
  FOR_EACH_OBSERVER(AnimationObserver, observers_, DidFinishAnimation(this));
}

void Animatable::DidRequestToFinishAnimation() {
}

void Animatable::MarkFinished() {
  DCHECK(!is_finished_);
  is_finished_ = true;
}

void Animatable::RemoveObserver(AnimationObserver* observer) {
  observers_.RemoveObserver(observer);
}

void Animatable::RequestToFinishAnimation() {
  DidRequestToFinishAnimation();
  DCHECK(is_finished_);
  if (animator_)
    return;
  DidFinishAnimation();
}

void Animatable::ScheduleAnimation() {
  DCHECK(animator_);
  DCHECK(!is_finished_);
  animator_->ScheduleAnimation(this);
}

}  // namespace ui
