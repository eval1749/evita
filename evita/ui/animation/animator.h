// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_ANIMATION_ANIMATOR_H_
#define EVITA_UI_ANIMATION_ANIMATOR_H_

#include <unordered_set>

#include "base/macros.h"
#include "base/time/time.h"
#include "common/memory/singleton.h"
#include "evita/ui/animation/animation_frame_handler.h"

namespace ui {

class Animatable;

class Animator : public common::Singleton<Animator>,
                 public AnimationFrameHandler {
  DECLARE_SINGLETON_CLASS(Animator);

 public:
  ~Animator() override;

  const base::TimeTicks& current_time() const;
  bool is_playing() const;

  void CancelAnimation(Animatable* animatable);
  void ScheduleAnimation(Animatable* animatable);

 private:
  Animator();

  void EndAnimate();
  void Animate(Animatable* animatable);
  void PlayAnimations(const base::TimeTicks& time);
  void StartAnimate(const base::TimeTicks& time);

  // AnimationFrameHandler
  void DidBeginAnimationFrame(const base::TimeTicks& time) override;
  const char* GetAnimationFrameType() const override;

  std::unordered_set<Animatable*> animatables_;
  base::TimeTicks current_time_;
  std::unordered_set<Animatable*> pending_animatables_;
  std::unordered_set<Animatable*> running_animatables_;
  std::unordered_set<Animatable*> waiting_animatables_;

  DISALLOW_COPY_AND_ASSIGN(Animator);
};

}  // namespace ui

#endif  // EVITA_UI_ANIMATION_ANIMATOR_H_
