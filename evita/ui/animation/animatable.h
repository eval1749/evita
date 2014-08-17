// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_animation_animatable_h)
#define INCLUDE_evita_ui_animation_animatable_h

#include "base/time/time.h"
#pragma warning(push)
#pragma warning(disable: 4625 4626)
#include "base/observer_list.h"
#pragma warning(pop)
#include "base/time/time.h"

namespace ui {

class AnimationObserver;
class Animator;

class Animatable {
  friend class Animator;

  private: Animator* animator_;
  private: bool is_finished_;
  private: ObserverList<AnimationObserver> observers_;

  protected: Animatable();
  public: virtual ~Animatable();

  public: Animator* animator() const { return animator_; }

  public: void AddObserver(AnimationObserver* observer);
  protected: virtual void Animate(base::Time time) = 0;
  public: void CancelAnimation();
  private: void DidCancel();
  protected: void DidAnimate();
  private: void DidFinish();
  public: void FinishAnimation();
  public: void RemoveObserver(AnimationObserver* observer);
  public: void ScheduleAnimation();

  DISALLOW_COPY_AND_ASSIGN(Animatable);
};

}   // namespace ui

#endif //!defined(INCLUDE_evita_ui_animation_animatable_h)
