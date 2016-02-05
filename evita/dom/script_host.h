// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_SCRIPT_HOST_H_
#define EVITA_DOM_SCRIPT_HOST_H_

#include <memory>
#include <string>
#include <vector>

#include "base/callback_forward.h"
#include "base/location.h"
#include "base/strings/string16.h"
#include "evita/dom/public/script_host_state.h"
#include "evita/dom/public/view_event_handler.h"
#include "evita/ginx/isolate_holder.h"
#include "evita/ginx/runner_delegate.h"
#include "evita/ginx/scoped_persistent.h"
#include "evita/ginx/v8.h"

namespace base {
class MessageLoop;
}

namespace domapi {
class IoDelegate;
class ViewDelegate;
}

namespace ginx {
class Runner;
}

namespace dom {

class Event;
class EventTarget;
class IdleTask;
class Performance;
class Scheduler;
class ViewEventHandlerImpl;

//////////////////////////////////////////////////////////////////////
//
// SuppressMessageBoxScope
//
class SuppressMessageBoxScope final {
 public:
  SuppressMessageBoxScope();
  ~SuppressMessageBoxScope();

 private:
  DISALLOW_COPY_AND_ASSIGN(SuppressMessageBoxScope);
};

//////////////////////////////////////////////////////////////////////
//
// ScriptHost
//
class ScriptHost final : public ginx::RunnerDelegate {
 public:
  ~ScriptHost() final;

  ViewEventHandlerImpl* event_handler() const { return event_handler_.get(); }
  static ScriptHost* instance();
  domapi::IoDelegate* io_delegate() const { return io_delegate_; }
  v8::Isolate* isolate() const;
  Performance* performance() const;
  ginx::Runner* runner() const;
  void set_testing_runner(ginx::Runner* runner);
  Scheduler* scheduler() const { return scheduler_; }
  domapi::ViewDelegate* view_delegate() const;

  // Call |handleEvent()| function in the class of |target| with |event|.
  void CallClassEventHandler(EventTarget* target, Event* event);
  static void CreateAndStart(Scheduler* scheduler,
                             domapi::ViewDelegate* view_delegate,
                             domapi::IoDelegate* io_delegate);
  void ResetForTesting();
  void RunMicrotasks();

  // Schedule idle task
  void ScheduleIdleTask(const base::Closure& task);

  static ScriptHost* StartForTesting(Scheduler* scheduler,
                                     domapi::ViewDelegate* view_delegate,
                                     domapi::IoDelegate* io_delegate);

  // Terminate script execution from another thread.
  void TerminateScriptExecution();
  void WillDestroyViewHost();

 private:
  ScriptHost(Scheduler* scheduler,
             domapi::ViewDelegate* view_delegate,
             domapi::IoDelegate* io_delegate);

  static ScriptHost* Create(Scheduler* scheduler,
                            domapi::ViewDelegate* view_delegate,
                            domapi::IoDelegate* io_delegate);

  void DidStartScriptHost();

  // ginx::RunnerDelegate
  v8::Local<v8::ObjectTemplate> GetGlobalTemplate(ginx::Runner* runner) final;
  void Start();
  void UnhandledException(ginx::Runner* runner,
                          const v8::TryCatch& try_catch) final;

  ginx::IsolateHolder isolate_holder_;
  std::unique_ptr<ViewEventHandlerImpl> event_handler_;
  domapi::IoDelegate* io_delegate_;
  // A |MessageLoop| where script runs on. We don't allow to run script other
  // than this message loop.
  base::MessageLoop* const message_loop_for_script_;
  std::unique_ptr<Performance> performance_;
  std::unique_ptr<ginx::Runner> runner_;
  Scheduler* scheduler_;
  domapi::ScriptHostState state_;
  bool testing_;
  ginx::Runner* testing_runner_;
  domapi::ViewDelegate* view_delegate_;

  DISALLOW_COPY_AND_ASSIGN(ScriptHost);
};

}  // namespace dom

#endif  // EVITA_DOM_SCRIPT_HOST_H_
