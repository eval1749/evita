// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_ANIMATION_ANIMATION_SCHEDULER_H_
#define EVITA_UI_ANIMATION_ANIMATION_SCHEDULER_H_

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
 public:
  explicit AnimationScheduler(base::MessageLoop* message_loop);
  ~AnimationScheduler();

  // Request animation frame.
  void CancelAnimationFrameRequest(AnimationFrameHandler* handler);
  // Request animation frame.
  void RequestAnimationFrame(AnimationFrameHandler* handler);

 private:
  enum class State;

  void HandleAnimationFrame(base::Time time);
  void Run();
  void Wait();

  std::unordered_set<AnimationFrameHandler*> canceled_handlers_;
  std::unique_ptr<base::Lock> lock_;
  base::MessageLoop* const message_loop_;
  State state_;
  std::unordered_set<AnimationFrameHandler*> pending_handlers_;

  DISALLOW_COPY_AND_ASSIGN(AnimationScheduler);
};

}  // namespace ui

#endif  // EVITA_UI_ANIMATION_ANIMATION_SCHEDULER_H_
