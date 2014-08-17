// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/animation/animatable.h"

#include "evita/ui/animation/animation_observer.h"

namespace ui {

Animatable::Animatable() : animator_(nullptr), is_finished_(false) {
}

Animatable::~Animatable() {
  DCHECK(!animator_);
}

void Animatable::AddObserver(AnimationObserver* observer) {
  observers_.AddObserver(observer);
}

void Animatable::DidAnimate() {
  DCHECK(animator_);
  FOR_EACH_OBSERVER(AnimationObserver, observers_, DidAnimate(this));
}

void Animatable::DidFinish() {
  DCHECK(animator_);
  FOR_EACH_OBSERVER(AnimationObserver, observers_, DidFinishAnimation(this));
}

void Animatable::FinishAnimation() {
  DCHECK(animator_);
  DCHECK(!is_finished_);
  is_finished_ = true;
}

void Animatable::RemoveObserver(AnimationObserver* observer) {
  observers_.RemoveObserver(observer);
}

}  // namespace ui
