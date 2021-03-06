// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/script_thread.h"

#include <utility>

#include "base/bind.h"
#include "base/callback.h"
#include "base/message_loop/message_loop.h"
#include "base/sys_info.h"
#include "base/task_runner.h"
#include "base/task_scheduler/task_scheduler.h"
#include "base/threading/thread.h"
#include "base/time/time.h"
#include "base/trace_event/trace_event.h"
#include "evita/dom/lock.h"
#include "evita/dom/public/text_composition_event.h"
#include "evita/dom/public/view_events.h"
#include "evita/dom/scheduler/scheduler_impl.h"
#include "evita/dom/script_host.h"
#include "evita/dom/view_event_handler_impl.h"
#include "evita/ui/events/event.h"

#define DCHECK_CALLED_ON_NON_SCRIPT_THREAD() \
  DCHECK_NE(thread_->message_loop(), base::MessageLoop::current())

#define DCHECK_CALLED_ON_SCRIPT_THREAD() \
  DCHECK_EQ(thread_->message_loop(), base::MessageLoop::current())

namespace dom {

namespace {
bool IsScriptBreakEvent(const domapi::KeyboardEvent& event) {
  if (!event.control_key)
    return false;
  return event.key_code == (static_cast<int>(ui::KeyCode::Pause) |
                            static_cast<int>(ui::Modifier::Control));
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// ScriptThread
//
ScriptThread::ScriptThread(domapi::ViewDelegate* view_delegate,
                           domapi::IoDelegate* io_delegate)
    : io_delegate_(io_delegate),
      scheduler_(new SchedulerImpl(this)),
      thread_(new base::Thread("script_thread")),
      view_delegate_(view_delegate) {}

ScriptThread::~ScriptThread() {}

Scheduler* ScriptThread::scheduler() const {
  return static_cast<Scheduler*>(scheduler_.get());
}

domapi::ViewEventHandler* ScriptThread::view_event_handler() const {
  return ScriptHost::instance()->event_handler();
}

void ScriptThread::ScheduleScriptTask(base::OnceClosure task) {
  DCHECK_CALLED_ON_NON_SCRIPT_THREAD();
  scheduler()->ScheduleTask(std::move(task));
  RequestAnimationFrame();
}

void ScriptThread::Start() {
  DCHECK_CALLED_ON_NON_SCRIPT_THREAD();
  thread_->Start();
  base::TaskScheduler::CreateAndStartWithDefaultParams("ScriptThread");
  scheduler_->Start(thread_->message_loop());
  thread_->message_loop()->task_runner()->PostTask(
      FROM_HERE, base::BindOnce(&ScriptHost::CreateAndStart,
                                base::Unretained(scheduler_.get()),
                                base::Unretained(view_delegate_),
                                base::Unretained(io_delegate_)));
}

// base::PingProvider
void ScriptThread::Ping(std::atomic<bool>* cookie) {
  thread_->message_loop()->task_runner()->PostTask(
      FROM_HERE,
      base::BindOnce(&ScriptThread::PingInternal, base::Unretained(cookie)));
}

// domapi::ViewEventHandler
#define DEFINE_VIEW_EVENT_HANDLER0(name)                                    \
  void ScriptThread::name() {                                               \
    DCHECK_CALLED_ON_NON_SCRIPT_THREAD();                                   \
    ScheduleScriptTask(                                                     \
        FROM_HERE, base::BindOnce(&ViewEventHandler::name,                  \
                                  base::Unretained(view_event_handler()))); \
  }

#define DEFINE_VIEW_EVENT_HANDLER1(name, type1)                               \
  void ScriptThread::name(type1 param1) {                                     \
    DCHECK_CALLED_ON_NON_SCRIPT_THREAD();                                     \
    ScheduleScriptTask(base::BindOnce(&ViewEventHandler::name,                \
                                      base::Unretained(view_event_handler()), \
                                      param1));                               \
  }

#define DEFINE_VIEW_EVENT_HANDLER2(name, type1, type2)                        \
  void ScriptThread::name(type1 param1, type2 param2) {                       \
    DCHECK_CALLED_ON_NON_SCRIPT_THREAD();                                     \
    ScheduleScriptTask(base::BindOnce(&ViewEventHandler::name,                \
                                      base::Unretained(view_event_handler()), \
                                      param1, param2));                       \
  }

#define DEFINE_VIEW_EVENT_HANDLER3(name, type1, type2, type3)                 \
  void ScriptThread::name(type1 param1, type2 param2, type3 param3) {         \
    DCHECK_CALLED_ON_NON_SCRIPT_THREAD();                                     \
    ScheduleScriptTask(base::BindOnce(&ViewEventHandler::name,                \
                                      base::Unretained(view_event_handler()), \
                                      param1, param2, param3));               \
  }

#define DEFINE_VIEW_EVENT_HANDLER4(name, type1, type2, type3, type4)          \
  void ScriptThread::name(type1 param1, type2 param2, type3 param3,           \
                          type4 param4) {                                     \
    DCHECK_CALLED_ON_NON_SCRIPT_THREAD();                                     \
    ScheduleScriptTask(base::BindOnce(&ViewEventHandler::name,                \
                                      base::Unretained(view_event_handler()), \
                                      param1, param2, param3, param4));       \
  }

#define DEFINE_VIEW_EVENT_HANDLER5(name, type1, type2, type3, type4, type5) \
  void ScriptThread::name(type1 param1, type2 param2, type3 param3,         \
                          type4 param4, type5 param5) {                     \
    DCHECK_CALLED_ON_NON_SCRIPT_THREAD();                                   \
    ScheduleScriptTask(base::BindOnce(                                      \
        &ViewEventHandler::name, base::Unretained(view_event_handler()),    \
        param1, param2, param3, param4, param5));                           \
  }

void ScriptThread::DidBeginFrame(const base::TimeTicks& deadline) {
  DCHECK_CALLED_ON_NON_SCRIPT_THREAD();
  animation_frame_request_count_ = 0;
  scheduler()->DidBeginFrame(deadline);
}

DEFINE_VIEW_EVENT_HANDLER1(DidActivateWindow, domapi::WindowId)
DEFINE_VIEW_EVENT_HANDLER5(DidChangeWindowBounds,
                           domapi::WindowId,
                           int,
                           int,
                           int,
                           int)
DEFINE_VIEW_EVENT_HANDLER2(DidChangeWindowVisibility,
                           domapi::WindowId,
                           domapi::Visibility)
DEFINE_VIEW_EVENT_HANDLER1(DidDestroyWindow, domapi::WindowId)
DEFINE_VIEW_EVENT_HANDLER2(DidDropWidget, domapi::WindowId, domapi::WindowId)

DEFINE_VIEW_EVENT_HANDLER1(DidRealizeWidget, domapi::WindowId)
DEFINE_VIEW_EVENT_HANDLER1(DispatchFocusEvent, const domapi::FocusEvent&)

void ScriptThread::DispatchKeyboardEvent(const domapi::KeyboardEvent& event) {
  DCHECK_CALLED_ON_NON_SCRIPT_THREAD();
  if (IsScriptBreakEvent(event)) {
    ScriptHost::instance()->TerminateScriptExecution();
    return;
  }
  ScheduleScriptTask(base::BindOnce(&ViewEventHandler::DispatchKeyboardEvent,
                                    base::Unretained(view_event_handler()),
                                    event));
}

// TODO(eval1749): Combine |MouseMove| events if last event is also
// |MouseMove| event.
DEFINE_VIEW_EVENT_HANDLER1(DispatchMouseEvent, const domapi::MouseEvent&)
DEFINE_VIEW_EVENT_HANDLER1(DispatchTextCompositionEvent,
                           const domapi::TextCompositionEvent&)
DEFINE_VIEW_EVENT_HANDLER1(DispatchWheelEvent, const domapi::WheelEvent&)

DEFINE_VIEW_EVENT_HANDLER2(OpenFile, domapi::WindowId, const base::string16&)
DEFINE_VIEW_EVENT_HANDLER2(ProcessCommandLine,
                           const base::string16&,
                           const std::vector<base::string16>&)
DEFINE_VIEW_EVENT_HANDLER1(QueryClose, domapi::WindowId)

void ScriptThread::RunCallback(base::OnceClosure callback) {
  DCHECK_CALLED_ON_NON_SCRIPT_THREAD();
  ScheduleScriptTask(base::BindOnce(&ViewEventHandler::RunCallback,
                                    base::Unretained(view_event_handler()),
                                    std::move(callback)));
}

void ScriptThread::WillDestroyViewHost() {
  DCHECK_CALLED_ON_NON_SCRIPT_THREAD();
  ScheduleScriptTask(base::BindOnce(&ViewEventHandler::WillDestroyViewHost,
                                    base::Unretained(view_event_handler())));
}

// SchedulerClient
void ScriptThread::DidCancelAnimationFrame() {
  DCHECK_CALLED_ON_SCRIPT_THREAD();
  DCHECK_GT(animation_frame_request_count_, 0);
  --animation_frame_request_count_;
  if (animation_frame_request_count_)
    return;
  CancelAnimationFrameRequest();
}

void ScriptThread::DidRequestAnimationFrame() {
  DCHECK_CALLED_ON_SCRIPT_THREAD();
  ++animation_frame_request_count_;
  if (animation_frame_request_count_ != 1)
    return;
  RequestAnimationFrame();
}

void ScriptThread::EnterIdle(const base::TimeTicks& deadline) {
  DCHECK_CALLED_ON_SCRIPT_THREAD();
  const auto remaining = (deadline - base::TimeTicks::Now()).InSecondsF();
  if (remaining <= 0)
    return;
  ScriptHost::instance()->isolate()->IdleNotificationDeadline(remaining);
}

void ScriptThread::RunMicrotasks() {
  ScriptHost::instance()->RunMicrotasks();
}

// ui::AnimationFrameHandler
const char* ScriptThread::GetAnimationFrameType() const {
  return "ScriptThread";
}

void ScriptThread::DidBeginAnimationFrame(const base::TimeTicks& time) {
  DCHECK_CALLED_ON_NON_SCRIPT_THREAD();
  // nothing to do
}

}  // namespace dom
