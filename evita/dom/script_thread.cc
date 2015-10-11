// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/script_thread.h"

#include "base/bind.h"
#include "base/callback.h"
#include "base/message_loop/message_loop.h"
#include "base/threading/thread.h"
#include "evita/dom/events/view_event_handler_impl.h"
#include "evita/dom/lock.h"
#include "evita/dom/public/text_composition_event.h"
#include "evita/dom/public/view_event.h"
#include "evita/dom/scheduler.h"
#include "evita/dom/script_host.h"
#include "evita/ui/events/event.h"
#include "v8/include/v8-debug.h"

#define DCHECK_CALLED_ON_NON_SCRIPT_THREAD() \
  DCHECK_NE(thread_->message_loop(), base::MessageLoop::current())

#define DCHECK_CALLED_ON_SCRIPT_THREAD() \
  DCHECK_EQ(thread_->message_loop(), base::MessageLoop::current())

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// ScriptThread
//
ScriptThread::ScriptThread(ViewDelegate* view_delegate,
                           domapi::IoDelegate* io_delegate)
    : io_delegate_(io_delegate),
      scheduler_(new Scheduler(view_delegate)),
      thread_(new base::Thread("script_thread")),
      view_delegate_(view_delegate) {}

ScriptThread::~ScriptThread() {}

domapi::ViewEventHandler* ScriptThread::view_event_handler() const {
  return ScriptHost::instance()->event_handler();
}

void ScriptThread::Start() {
  DCHECK_CALLED_ON_NON_SCRIPT_THREAD();
  thread_->Start();
  thread_->message_loop()->PostTask(
      FROM_HERE,
      base::Bind(&ScriptHost::CreateAndStart, base::Unretained(view_delegate_),
                 base::Unretained(io_delegate_)));
}

// domapi::ViewEventHandler
#define DEFINE_VIEW_EVENT_HANDLER0(name)                                \
  void ScriptThread::name() {                                           \
    DCHECK_CALLED_ON_NON_SCRIPT_THREAD();                               \
    thread_->message_loop()->PostTask(                                  \
        FROM_HERE, base::Bind(&ViewEventHandler::name,                  \
                              base::Unretained(view_event_handler()))); \
  }

#define DEFINE_VIEW_EVENT_HANDLER1(name, type1)                      \
  void ScriptThread::name(type1 param1) {                            \
    DCHECK_CALLED_ON_NON_SCRIPT_THREAD();                            \
    scheduler_->ScheduleTask(                                        \
        base::Bind(&ViewEventHandler::name,                          \
                   base::Unretained(view_event_handler()), param1)); \
  }

#define DEFINE_VIEW_EVENT_HANDLER2(name, type1, type2)                       \
  void ScriptThread::name(type1 param1, type2 param2) {                      \
    DCHECK_CALLED_ON_NON_SCRIPT_THREAD();                                    \
    scheduler_->ScheduleTask(                                                \
        base::Bind(&ViewEventHandler::name,                                  \
                   base::Unretained(view_event_handler()), param1, param2)); \
  }

#define DEFINE_VIEW_EVENT_HANDLER3(name, type1, type2, type3)            \
  void ScriptThread::name(type1 param1, type2 param2, type3 param3) {    \
    DCHECK_CALLED_ON_NON_SCRIPT_THREAD();                                \
    scheduler_->ScheduleTask(base::Bind(                                 \
        &ViewEventHandler::name, base::Unretained(view_event_handler()), \
        param1, param2, param3));                                        \
  }

#define DEFINE_VIEW_EVENT_HANDLER4(name, type1, type2, type3, type4)     \
  void ScriptThread::name(type1 param1, type2 param2, type3 param3,      \
                          type4 param4) {                                \
    DCHECK_CALLED_ON_NON_SCRIPT_THREAD();                                \
    scheduler_->ScheduleTask(base::Bind(                                 \
        &ViewEventHandler::name, base::Unretained(view_event_handler()), \
        param1, param2, param3, param4));                                \
  }

#define DEFINE_VIEW_EVENT_HANDLER5(name, type1, type2, type3, type4, type5) \
  void ScriptThread::name(type1 param1, type2 param2, type3 param3,         \
                          type4 param4, type5 param5) {                     \
    DCHECK_CALLED_ON_NON_SCRIPT_THREAD();                                   \
    scheduler_->ScheduleTask(base::Bind(                                    \
        &ViewEventHandler::name, base::Unretained(view_event_handler()),    \
        param1, param2, param3, param4, param5));                           \
  }

void ScriptThread::DidBeginFrame(const base::Time& deadline) {
  thread_->message_loop()->PostTask(
      FROM_HERE, base::Bind(&Scheduler::DidBeginFrame,
                            base::Unretained(scheduler_.get()), deadline));
}

DEFINE_VIEW_EVENT_HANDLER5(DidChangeWindowBounds, WindowId, int, int, int, int)
DEFINE_VIEW_EVENT_HANDLER2(DidChangeWindowVisibility,
                           WindowId,
                           domapi::Visibility)
DEFINE_VIEW_EVENT_HANDLER1(DidDestroyWidget, WindowId)
DEFINE_VIEW_EVENT_HANDLER2(DidDropWidget, WindowId, WindowId)
DEFINE_VIEW_EVENT_HANDLER1(DidRealizeWidget, WindowId)
DEFINE_VIEW_EVENT_HANDLER1(DispatchFocusEvent, const domapi::FocusEvent&)

void ScriptThread::DispatchKeyboardEvent(const domapi::KeyboardEvent& event) {
  DCHECK_CALLED_ON_NON_SCRIPT_THREAD();
  if (event.key_code == (static_cast<int>(ui::KeyCode::Pause) |
                         static_cast<int>(ui::Modifier::Control)) &&
      event.control_key) {
    // We should have |ScriptHost::TerminateExecution()| instead of calling
    // |v8::Isolate::TerminateExecution()| here.
    auto const isolate = ScriptHost::instance()->isolate();
    isolate->TerminateExecution();
    return;
  }

  scheduler_->ScheduleTask(base::Bind(&ViewEventHandler::DispatchKeyboardEvent,
                                      base::Unretained(view_event_handler()),
                                      event));
}

DEFINE_VIEW_EVENT_HANDLER1(DispatchMouseEvent, const domapi::MouseEvent&)
DEFINE_VIEW_EVENT_HANDLER1(DispatchTextCompositionEvent,
                           const domapi::TextCompositionEvent&)
DEFINE_VIEW_EVENT_HANDLER1(DispatchWheelEvent, const domapi::WheelEvent&)
DEFINE_VIEW_EVENT_HANDLER2(OpenFile, WindowId, const base::string16&)
DEFINE_VIEW_EVENT_HANDLER2(ProcessCommandLine,
                           const base::string16&,
                           const std::vector<base::string16>&)
DEFINE_VIEW_EVENT_HANDLER1(QueryClose, WindowId)
DEFINE_VIEW_EVENT_HANDLER1(RunCallback, const base::Closure&)

void ScriptThread::WillDestroyViewHost() {
  DCHECK_CALLED_ON_NON_SCRIPT_THREAD();
  scheduler_->ScheduleTask(base::Bind(&ViewEventHandler::WillDestroyViewHost,
                                      base::Unretained(view_event_handler())));
}

}  // namespace dom
