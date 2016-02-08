// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_SCRIPT_THREAD_H_
#define EVITA_DOM_SCRIPT_THREAD_H_

#include <memory>
#include <string>
#include <vector>

#include "base/callback_forward.h"
#include "base/macros.h"
#include "evita/base/ping_provider.h"
#include "evita/dom/public/view_delegate.h"
#include "evita/dom/public/view_event_handler.h"
#include "evita/dom/scheduler_client.h"
#include "evita/ui/animation/animation_frame_handler.h"

namespace base {
class Thread;
}

namespace domapi {
class IoDelegate;
}

namespace dom {

class Scheduler;
class SchedulerImpl;

class ScriptThread final : public base::PingProvider,
                           public domapi::ViewEventHandler,
                           public SchedulerClient,
                           public ui::AnimationFrameHandler {
 public:
  ScriptThread(domapi::ViewDelegate* view_delegate,
               domapi::IoDelegate* io_delegate);
  ~ScriptThread() final;

  void Start();

 private:
  Scheduler* scheduler() const;
  domapi::ViewEventHandler* view_event_handler() const;

  void BeginAnimationFrame(const base::TimeTicks& time);
  void ScheduleScriptTask(const base::Closure& task);

  // base::PingProvider
  void Ping(std::atomic<bool>* cookie) final;

  // domapi::ViewEventHandler
  void DidActivateWindow(domapi::WindowId window_id) final;
  void DidBeginFrame(const base::TimeTicks& deadline);
  void DidChangeWindowBounds(domapi::WindowId window_id,
                             int left,
                             int top,
                             int right,
                             int bottom) final;
  void DidChangeWindowVisibility(domapi::WindowId window_id,
                                 domapi::Visibility visibility) final;
  void DidDestroyWindow(domapi::WindowId window_id) final;
  void DidDropWidget(domapi::WindowId source_id,
                     domapi::WindowId target_id) final;
  void DidRealizeWidget(domapi::WindowId window_id) final;
  void DidEnterViewIdle(const base::TimeTicks& deadline) final;
  void DidExitViewIdle() final;
  void DispatchFocusEvent(const domapi::FocusEvent& event) final;
  void DispatchKeyboardEvent(const domapi::KeyboardEvent& event) final;
  void DispatchMouseEvent(const domapi::MouseEvent& event) final;
  void DispatchTextCompositionEvent(
      const domapi::TextCompositionEvent& event) final;
  void DispatchWheelEvent(const domapi::WheelEvent& event) final;
  void OpenFile(domapi::WindowId window_id,
                const base::string16& file_name) final;
  void ProcessCommandLine(const base::string16& working_directory,
                          const std::vector<base::string16>& args) final;
  void QueryClose(domapi::WindowId window_id) final;
  void RunCallback(const base::Closure& callback) final;
  void WillDestroyViewHost() final;

  // SchedulerClient
  void DidCancelAnimationFrame() final;
  void DidUpdateDom() final;
  void DidRequestAnimationFrame() final;

  // ui::AnimationFrameHandler
  const char* GetAnimationFrameType() const final;
  void DidBeginAnimationFrame(const base::TimeTicks& time) final;

  int animation_frame_request_count_ = 0;
  domapi::IoDelegate* const io_delegate_;
  const std::unique_ptr<SchedulerImpl> scheduler_;
  const std::unique_ptr<base::Thread> thread_;
  domapi::ViewDelegate* const view_delegate_;

  DISALLOW_COPY_AND_ASSIGN(ScriptThread);
};

}  // namespace dom

#endif  // EVITA_DOM_SCRIPT_THREAD_H_
