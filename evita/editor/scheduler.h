// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_EDITOR_SCHEDULER_H_
#define EVITA_EDITOR_SCHEDULER_H_

#include <memory>
#include <unordered_set>

#include "base/basictypes.h"
#include "base/time/time.h"

namespace base {
class Lock;
class MessageLoop;
}

namespace ui {
class AnimationFrameHandler;
}

namespace editor {

//////////////////////////////////////////////////////////////////////
//
// Scheduler
//
class Scheduler final {
 public:
  explicit Scheduler(base::MessageLoop* message_loop);
  ~Scheduler();

  // Request animation frame.
  void CancelAnimationFrameRequest(ui::AnimationFrameHandler* handler);

  // Called when DOM thread stopped.
  void DidUpdateDom();

  // Request animation frame.
  void RequestAnimationFrame(ui::AnimationFrameHandler* handler);

 private:
  void DidFireTimer();
  void HandleAnimationFrame(base::Time time);
  void RunAnimation();
  void StartTimerIfNeeded();

  std::unordered_set<ui::AnimationFrameHandler*> canceled_handlers_;
  base::Time last_paint_time_;
  std::unique_ptr<base::Lock> lock_;
  base::MessageLoop* const message_loop_;
  std::unordered_set<ui::AnimationFrameHandler*> pending_handlers_;
  bool script_is_running_;
  base::Time script_start_time_;
  bool timer_is_running_;

  DISALLOW_COPY_AND_ASSIGN(Scheduler);
};

}  // namespace editor

#endif  // EVITA_EDITOR_SCHEDULER_H_
