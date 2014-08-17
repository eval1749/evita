// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/animation/animatables.h"

#include "base/logging.h"
#include "evita/ui/animation/animatable.h"
#include "evita/ui/animation/animator.h"

namespace ui {

Animatables::Animatables() {
}

Animatables::~Animatables() {
  for (auto animatable : animatables_) {
    if (auto const animator = animatable->animator())
      animator->CancelAnimation(animatable);
    delete animatable;
  }
}

void Animatables::AddAnimatable(Animatable* animatable) {
  DCHECK(animatables_.find(animatable) == animatables_.end());
  animatable->AddObserver(this);
  animatables_.insert(animatable);
}

// ui::AnimationObserver
void Animatables::DidFinishAnimation(ui::Animatable* animatable) {
  animatable->RemoveObserver(this);
  animatables_.erase(animatable);
}

}  // namespace ui
