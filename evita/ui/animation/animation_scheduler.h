// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_animation_animation_scheduler_h)
#define INCLUDE_evita_ui_animation_animation_scheduler_h

#include "base/basictypes.h"
#include "common/memory/singleton.h"

#include <unordered_set>

namespace base {
class Time;
}

namespace ui {

class AnimationFrameHandler;

//////////////////////////////////////////////////////////////////////
//
// AnimationScheduler
//
class AnimationScheduler final : public common::Singleton<AnimationScheduler> {
  DECLARE_SINGLETON_CLASS(AnimationScheduler);

  private: std::unordered_set<AnimationFrameHandler*> canceled_handlers_;
  private: std::unordered_set<AnimationFrameHandler*> pending_handlers_;

  private: AnimationScheduler();
  public: virtual ~AnimationScheduler();

  // Request animation frame.
  public: void CancelAnimationFrameRequest(AnimationFrameHandler* handler);

  public: void HandleAnimationFrame(base::Time time);

  // Request animation frame.
  public: void RequestAnimationFrame(AnimationFrameHandler* handler);

  DISALLOW_COPY_AND_ASSIGN(AnimationScheduler);
};

}   // namespace ui

#endif //!defined(INCLUDE_evita_ui_animation_animation_scheduler_h)
