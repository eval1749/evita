// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_animation_animator_h)
#define INCLUDE_evita_ui_animation_animator_h

#include "base/basictypes.h"
#include "base/time/time.h"
#include "common/memory/singleton.h"

#include <unordered_set>

namespace ui {

class Animatable;

class Animator : public common::Singleton<Animator> {
  DECLARE_SINGLETON_CLASS(Animator);

  private: std::unordered_set<Animatable*> animatables_;
  private: base::Time current_time_;
  private: std::unordered_set<Animatable*> pending_animatables_;
  private: std::unordered_set<Animatable*> running_animatables_;
  private: std::unordered_set<Animatable*> waiting_animatables_;

  private: Animator();
  public: virtual ~Animator();

  public: base::Time current_time() const;
  public: bool is_playing() const;

  private: void Animate(Animatable* animatable);
  public: void CancelAnimation(Animatable* animatable);
  private: void EndAnimate();
  public: void PlayAnimation(base::Time time, Animatable* animatable);
  public: void PlayAnimations(base::Time time);
  public: void ScheduleAnimation(Animatable* animatable);
  private: void StartAnimate(base::Time time);

  DISALLOW_COPY_AND_ASSIGN(Animator);
};

}   // namespace ui

#endif //!defined(INCLUDE_evita_ui_animation_animator_h)
