// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/animation/animation_observer.h"

namespace ui {

AnimationObserver::AnimationObserver() {
}

AnimationObserver::~AnimationObserver() {
}

void AnimationObserver::DidAnimate(Animatable*) {
}

void AnimationObserver::DidFinishAnimation(Animatable*) {
}

}  // namespace ui
