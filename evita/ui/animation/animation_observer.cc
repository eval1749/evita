// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/animation/animation_observer.h"

#include "base/logging.h"

namespace ui {

AnimationObserver::AnimationObserver() {}

AnimationObserver::~AnimationObserver() {}

void AnimationObserver::DidAnimate(Animatable* animatable) {
  DCHECK(animatable);
}

void AnimationObserver::DidCancelAnimation(Animatable* animatable) {
  DCHECK(animatable);
}

void AnimationObserver::DidFinishAnimation(Animatable* animatable) {
  DCHECK(animatable);
}

}  // namespace ui
