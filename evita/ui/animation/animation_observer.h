// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_ANIMATION_ANIMATION_OBSERVER_H_
#define EVITA_UI_ANIMATION_ANIMATION_OBSERVER_H_

#include "base/macros.h"

namespace ui {

class Animatable;

class AnimationObserver {
 public:
  virtual ~AnimationObserver();

  virtual void DidAnimate(Animatable* animatable);
  virtual void DidCancelAnimation(Animatable* animatable);
  virtual void DidFinishAnimation(Animatable* animatable);

 protected:
  AnimationObserver();

 private:
  DISALLOW_COPY_AND_ASSIGN(AnimationObserver);
};

}  // namespace ui

#endif  // EVITA_UI_ANIMATION_ANIMATION_OBSERVER_H_
