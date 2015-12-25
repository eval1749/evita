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
#include "evita/dom/public/view_delegate.h"
#include "evita/dom/public/view_event_handler.h"
#include "evita/dom/scheduler_client.h"

namespace base {
class Thread;
}

namespace domapi {
class IoDelegate;
}

namespace dom {

class Scheduler;
class SchedulerImpl;

class ScriptThread final : public domapi::ViewEventHandler,
                           public SchedulerClient {
 public:
  ScriptThread(domapi::ViewDelegate* view_delegate,
               domapi::IoDelegate* io_delegate);
  ~ScriptThread() final;

  void Start();

 private:
  Scheduler* scheduler() const;
  domapi::ViewEventHandler* view_event_handler() const;

  void ScheduleScriptTask(const base::Closure& task);

  // domapi::ViewEventHandler
  void DidBeginFrame(const base::Time& deadline);
  void DidChangeWindowBounds(domapi::WindowId window_id,
                             int left,
                             int top,
                             int right,
                             int bottom) final;
  void DidChangeWindowVisibility(domapi::WindowId window_id,
                                 domapi::Visibility visibility) final;
  void DidDestroyWidget(domapi::WindowId window_id) final;
  void DidDropWidget(domapi::WindowId source_id,
                     domapi::WindowId target_id) final;
  void DidRealizeWidget(domapi::WindowId window_id) final;
  void DidEnterViewIdle(const base::Time& deadline) final;
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
  void DidUpdateDom() final;

  domapi::IoDelegate* const io_delegate_;
  const std::unique_ptr<SchedulerImpl> scheduler_;
  const std::unique_ptr<base::Thread> thread_;
  domapi::ViewDelegate* const view_delegate_;

  DISALLOW_COPY_AND_ASSIGN(ScriptThread);
};

}  // namespace dom

#endif  // EVITA_DOM_SCRIPT_THREAD_H_
