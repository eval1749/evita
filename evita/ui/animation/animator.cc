// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/animation/animator.h"

#include <vector>

#include "base/logging.h"
#include "base/time/time.h"
#include "evita/ui/animation/animatable.h"

namespace ui {

Animator::Animator() {
}

Animator::~Animator() {
}

void Animator::Animate(base::Time time) {
  std::vector<Animatable*> animatables(animatables_.size());
  animatables.resize(0);
  for (auto animatable : animatables_) {
    animatables.push_back(animatable);
  }
  animatables_.clear();
  for (auto animatable : animatables) {
    animatable->Animate(time);
  }
}

void Animator::CancelAnimation(Animatable* animatable) {
  auto const present = animatables_.find(animatable);
  if (present == animatables_.end())
    return;
  animatables_.erase(present);
}

void Animator::PlayAnimation(base::Time now, Animatable* animatable) {
  CancelAnimation(animatable);
  animatable->Animate(now);
}

void Animator::ScheduleAnimation(Animatable* animatable) {
  animatables_.insert(animatable);
}

}  // namespace ui
