// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/animation/animatable.h"

#include "evita/ui/animation/animation_observer.h"

namespace ui {

Animatable::Animatable() : animator_(nullptr), state_(State::NotScheduled) {
}

Animatable::~Animatable() {
  DCHECK(!animator_);
  DCHECK_EQ(State::NotScheduled, state_);
}

void Animatable::AddObserver(AnimationObserver* observer) {
  observers_.AddObserver(observer);
}

void Animatable::DidAnimate() {
  DCHECK(animator_);
  FOR_EACH_OBSERVER(AnimationObserver, observers_, DidAnimate(this));
}

void Animatable::FinishAnimation() {
  DCHECK(animator_);
  DCHECK_EQ(State::Playing, state_);
  state_ = State::Finished;
}

void Animatable::RemoveObserver(AnimationObserver* observer) {
  observers_.RemoveObserver(observer);
}

}  // namespace ui

std::ostream& operator<<(std::ostream& ostream, ui::Animatable::State state) {
  switch (state) {
    case ui::Animatable::State::Finished:
      return ostream << "Finished";
    case ui::Animatable::State::NotScheduled:
      return ostream << "NotScheduled";
    case ui::Animatable::State::Playing:
      return ostream << "Playing";
    case ui::Animatable::State::Scheduled:
      return ostream << "Scheduled";
    default:
      return ostream << "Unknown";
  }
}
