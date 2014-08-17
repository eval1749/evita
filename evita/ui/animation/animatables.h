// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_animation_animatables_h)
#define INCLUDE_evita_ui_animation_animatables_h

#include <unordered_set>

#include "base/basictypes.h"
#include "evita/ui/animation/animation_observer.h"

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// Animatables
// Manages life time of |Animatable| objects.
//
class Animatables final : public AnimationObserver {
  private: std::unordered_set<Animatable*> animatables_;

  public: Animatables();
  public: ~Animatables();

  public: void AddAnimatable(Animatable* animatables);

  // AnimationObserver
  private: virtual void DidFinishAnimation(Animatable* animatable) override;

  DISALLOW_COPY_AND_ASSIGN(Animatables);
};

}   // namespace ui

#endif //!defined(INCLUDE_evita_ui_animation_animatables_h)
