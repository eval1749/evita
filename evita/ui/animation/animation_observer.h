// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_animation_animation_observer)
#define INCLUDE_evita_ui_animation_animation_observer

#include "base/basictypes.h"

namespace ui {

class Animatable;

class AnimationObserver {
  protected: AnimationObserver();
  public: virtual ~AnimationObserver();

  public: virtual void DidCancelAnimation(Animatable* animatable);
  public: virtual void DidAnimate(Animatable* animatable);
  public: virtual void DidFinishAnimation(Animatable* animatable);

  DISALLOW_COPY_AND_ASSIGN(AnimationObserver);
};

}   // namespace ui

#endif //!defined(INCLUDE_evita_ui_animation_animation_observer)
