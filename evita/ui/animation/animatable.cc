// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/animation/animatable.h"

#include "evita/ui/animation/animation_observer.h"

namespace ui {

Animatable::Animatable() {
}

Animatable::~Animatable() {
}

void Animatable::AddObserver(AnimationObserver* observer) {
  observers_.AddObserver(observer);
}

void Animatable::DidAnimate() {
  FOR_EACH_OBSERVER(AnimationObserver, observers_, DidAnimate(this));
}

void Animatable::FinishAnimation() {
  FOR_EACH_OBSERVER(AnimationObserver, observers_, DidFinishAnimation(this));
}

void Animatable::PlayAnimation(base::Time time) {
  Animate(time);
}

void Animatable::RemoveObserver(AnimationObserver* observer) {
  observers_.RemoveObserver(observer);
}

}  // namespace ui
