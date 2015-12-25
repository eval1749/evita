// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_ANIMATION_ANIMATABLES_H_
#define EVITA_UI_ANIMATION_ANIMATABLES_H_

#include <unordered_set>

#include "base/macros.h"
#include "evita/ui/animation/animation_observer.h"

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// Animatables
// Manages life time of |Animatable| objects.
//
class Animatables final : public AnimationObserver {
 public:
  Animatables();
  ~Animatables() final;

  void AddAnimatable(Animatable* animatables);

 private:
  // AnimationObserver
  void DidFinishAnimation(Animatable* animatable) final;

  std::unordered_set<Animatable*> animatables_;

  DISALLOW_COPY_AND_ASSIGN(Animatables);
};

}  // namespace ui

#endif  // EVITA_UI_ANIMATION_ANIMATABLES_H_
