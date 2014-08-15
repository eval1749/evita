// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_animation_animator_h)
#define INCLUDE_evita_ui_animation_animator_h

#include "base/basictypes.h"
#include "common/memory/singleton.h"

#include <unordered_set>

namespace base {
class Time;
}

namespace ui {

class Animatable;

class Animator : public common::Singleton<Animator> {
  DECLARE_SINGLETON_CLASS(Animator);

  private: std::unordered_set<Animatable*> animatables_;

  private: Animator();
  public: virtual ~Animator();

  public: void Animate(base::Time time);
  public: void CancelAnimation(Animatable* animatable);
  public: void PlayAnimation(base::Time time, Animatable* animatable);
  public: void ScheduleAnimation(Animatable* animatable);

  DISALLOW_COPY_AND_ASSIGN(Animator);
};

}   // namespace ui

#endif //!defined(INCLUDE_evita_ui_animation_animator_h)
