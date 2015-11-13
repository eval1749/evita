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

namespace domapi {
class ViewEventHandler;
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
  enum class State;

  explicit Scheduler(domapi::ViewEventHandler* script_delegate);
  ~Scheduler();

  // Request animation frame.
  void CancelAnimationFrameRequest(ui::AnimationFrameHandler* handler);

  // Called when DOM thread stopped.
  void DidUpdateDom();

  // Request animation frame.
  void RequestAnimationFrame(ui::AnimationFrameHandler* handler);

  void Start();

 private:
  void BeginFrame();
  void ChangeState(State new_state);
  void CommitFrame();
  void EnterIdle();
  void ExitIdle();
  void HandleAnimationFrame();
  void ScheduleNextFrame();

  std::unordered_set<ui::AnimationFrameHandler*> canceled_handlers_;
  int idle_sequence_num_;
  base::Time last_frame_time_;
  base::Time last_paint_time_;
  std::unique_ptr<base::Lock> lock_;
  base::MessageLoop* const message_loop_;
  std::unordered_set<ui::AnimationFrameHandler*> pending_handlers_;
  domapi::ViewEventHandler* script_delegate_;
  State state_;
  int state_sequence_num_;

  DISALLOW_COPY_AND_ASSIGN(Scheduler);
};

}  // namespace editor

#endif  // EVITA_EDITOR_SCHEDULER_H_
