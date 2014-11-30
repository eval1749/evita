// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_animation_animation_scheduler_h)
#define INCLUDE_evita_ui_animation_animation_scheduler_h

#include <memory>
#include <unordered_set>

#include "base/basictypes.h"

namespace base {
class Lock;
class MessageLoop;
class Time;
}

namespace ui {

class AnimationFrameHandler;

//////////////////////////////////////////////////////////////////////
//
// AnimationScheduler
//
class AnimationScheduler final {
  private: enum class State;

  private: std::unordered_set<AnimationFrameHandler*> canceled_handlers_;
  private: std::unique_ptr<base::Lock> lock_;
  private: base::MessageLoop* const message_loop_;
  private: State state_;
  private: std::unordered_set<AnimationFrameHandler*> pending_handlers_;

  public: explicit AnimationScheduler(base::MessageLoop* message_loop);
  public: ~AnimationScheduler();

  // Request animation frame.
  public: void CancelAnimationFrameRequest(AnimationFrameHandler* handler);
  private: void HandleAnimationFrame(base::Time time);
  // Request animation frame.
  public: void RequestAnimationFrame(AnimationFrameHandler* handler);
  private: void Run();
  private: void Wait();

  DISALLOW_COPY_AND_ASSIGN(AnimationScheduler);
};

}   // namespace ui

#endif //!defined(INCLUDE_evita_ui_animation_animation_scheduler_h)
