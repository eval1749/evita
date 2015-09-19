// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_ANIMATION_ANIMATABLE_H_
#define EVITA_UI_ANIMATION_ANIMATABLE_H_

#include "base/observer_list.h"
#include "base/time/time.h"

namespace ui {

class AnimationObserver;
class Animator;

class Animatable {
 public:
  virtual ~Animatable();

  Animator* animator() const { return animator_; }

  void AddObserver(AnimationObserver* observer);
  void CancelAnimation();
  void FinishAnimation();
  void RemoveObserver(AnimationObserver* observer);
  void ScheduleAnimation();

 protected:
  Animatable();
  virtual void Animate(base::Time time) = 0;
  void DidAnimate();
  virtual void FinalizeAnimation();

 private:
  friend class Animator;

  void DidCancelAnimation();
  void DidFinishAnimation();

  Animator* animator_;
  bool is_finished_;
  base::ObserverList<AnimationObserver> observers_;

  DISALLOW_COPY_AND_ASSIGN(Animatable);
};

}  // namespace ui

#endif  // EVITA_UI_ANIMATION_ANIMATABLE_H_
