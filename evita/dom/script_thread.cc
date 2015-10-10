// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/script_thread.h"

#pragma warning(push)
#pragma warning(disable : 4100 4625 4626)
#include "base/bind.h"
#include "base/callback.h"
#include "base/message_loop/message_loop.h"
#include "base/synchronization/waitable_event.h"
#include "base/threading/thread.h"
#pragma warning(pop)
#include "evita/dom/events/view_event_handler_impl.h"
#include "evita/dom/lock.h"
#include "evita/dom/public/float_rect.h"
#include "evita/dom/public/tab_data.h"
#include "evita/dom/public/text_composition_event.h"
#include "evita/dom/public/view_event.h"
#include "evita/dom/script_host.h"
#include "evita/ui/events/event.h"
#include "v8/include/v8-debug.h"

#define DCHECK_CALLED_ON_NON_SCRIPT_THREAD() \
  DCHECK_NE(thread_->message_loop(), base::MessageLoop::current())

#define DCHECK_CALLED_ON_SCRIPT_THREAD() \
  DCHECK_EQ(thread_->message_loop(), base::MessageLoop::current())

namespace dom {

namespace {

//////////////////////////////////////////////////////////////////////
//
// SynchronousCaller
// Run task on another thread and wait for task done.
//
template <typename Result, typename... Params>
class SynchronousCaller final {
 public:
  SynchronousCaller(const base::Callback<Result(Params...)>& task,
                    base::WaitableEvent* event)
      : event_(event), task_(task) {}

  ~SynchronousCaller() = default;

  Result Call(base::MessageLoop* message_loop) {
    event_->Reset();
    DOM_AUTO_UNLOCK_SCOPE();
    message_loop->PostTask(FROM_HERE, base::Bind(&SynchronousCaller::RunTask,
                                                 base::Unretained(this)));
    event_->Wait();
    return result_;
  }

 private:
  void RunTask() {
    result_ = task_.Run();
    event_->Signal();
  }

  base::Callback<Result(Params...)> task_;
  base::WaitableEvent* event_;
  Result result_;

  DISALLOW_COPY_AND_ASSIGN(SynchronousCaller);
};

template <typename Result, typename... Params>
Result DoSynchronousCall(const base::Callback<Result(Params...)>& task,
                         base::MessageLoop* message_loop,
                         base::WaitableEvent* event) {
  SynchronousCaller<Result, Params...> caller(task, event);
  return caller.Call(message_loop);
}

//////////////////////////////////////////////////////////////////////
//
// SynchronousRunner
// Run task on another thread and wait for task done.
//
template <typename... Params>
class SynchronousRunner final {
 public:
  SynchronousRunner(const base::Callback<void(Params...)>& task,
                    base::WaitableEvent* event)
      : event_(event), task_(task) {}

  ~SynchronousRunner() = default;

  void Run(base::MessageLoop* message_loop) {
    event_->Reset();
    DOM_AUTO_UNLOCK_SCOPE();
    message_loop->PostTask(FROM_HERE, base::Bind(&SynchronousRunner::RunTask,
                                                 base::Unretained(this)));
    event_->Wait();
  }

 private:
  void RunTask() {
    task_.Run();
    event_->Signal();
  }

  base::Callback<void(Params...)> task_;
  base::WaitableEvent* event_;

  DISALLOW_COPY_AND_ASSIGN(SynchronousRunner);
};

template <typename... Params>
void RunSynchronously(const base::Callback<void(Params...)>& task,
                      base::MessageLoop* message_loop,
                      base::WaitableEvent* event) {
  SynchronousRunner<Params...> caller(task, event);
  caller.Run(message_loop);
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// ScriptThread
//
ScriptThread::ScriptThread(ViewDelegate* view_delegate,
                           domapi::IoDelegate* io_delegate)
    : io_delegate_(io_delegate),
      thread_(std::make_unique<base::Thread>("script_thread")),
      view_delegate_(view_delegate),
      view_message_loop_(nullptr),
      waitable_event_(new base::WaitableEvent(true, false)),
      script_host_(ScriptHost::Create(this, io_delegate)) {}

ScriptThread::~ScriptThread() {}

domapi::ViewEventHandler* ScriptThread::view_event_handler() const {
  return script_host_->event_handler();
}

void ScriptThread::Start(base::MessageLoop* view_message_loop) {
  DCHECK(view_message_loop);
  DCHECK(!view_message_loop_);
  DCHECK_CALLED_ON_NON_SCRIPT_THREAD();
  view_message_loop_ = view_message_loop;
  thread_->Start();
  thread_->message_loop()->PostTask(
      FROM_HERE,
      base::Bind(&ScriptHost::Start, base::Unretained(script_host_.get())));
}

// ViewDelegate
#define DEFINE_VIEW_DELEGATE_1(name, type1)                               \
  void ScriptThread::name(type1 param1) {                                 \
    DCHECK_CALLED_ON_SCRIPT_THREAD();                                     \
    if (!view_message_loop_)                                              \
      return;                                                             \
    view_message_loop_->PostTask(                                         \
        FROM_HERE, base::Bind(&ViewDelegate::name,                        \
                              base::Unretained(view_delegate_), param1)); \
  }

#define DEFINE_VIEW_DELEGATE_2(name, type1, type2)                        \
  void ScriptThread::name(type1 param1, type2 param2) {                   \
    DCHECK_CALLED_ON_SCRIPT_THREAD();                                     \
    if (!view_message_loop_)                                              \
      return;                                                             \
    view_message_loop_->PostTask(                                         \
        FROM_HERE,                                                        \
        base::Bind(&ViewDelegate::name, base::Unretained(view_delegate_), \
                   param1, param2));                                      \
  }

#define DEFINE_VIEW_DELEGATE_3(name, type1, type2, type3)                 \
  void ScriptThread::name(type1 param1, type2 param2, type3 param3) {     \
    DCHECK_CALLED_ON_SCRIPT_THREAD();                                     \
    if (!view_message_loop_)                                              \
      return;                                                             \
    view_message_loop_->PostTask(                                         \
        FROM_HERE,                                                        \
        base::Bind(&ViewDelegate::name, base::Unretained(view_delegate_), \
                   param1, param2, param3));                              \
  }

#define DEFINE_VIEW_DELEGATE_4(name, type1, type2, type3, type4)              \
  void ScriptThread::name(type1 p1, type2 p2, type3 p3, type4 p4) {           \
    DCHECK_CALLED_ON_SCRIPT_THREAD();                                         \
    if (!view_message_loop_)                                                  \
      return;                                                                 \
    view_message_loop_->PostTask(                                             \
        FROM_HERE,                                                            \
        base::Bind(&ViewDelegate::name, base::Unretained(view_delegate_), p1, \
                   p2, p3, p4));                                              \
  }

#define DEFINE_VIEW_DELEGATE_5(name, type1, type2, type3, type4, type5)       \
  void ScriptThread::name(type1 p1, type2 p2, type3 p3, type4 p4, type5 p5) { \
    DCHECK_CALLED_ON_SCRIPT_THREAD();                                         \
    if (!view_message_loop_)                                                  \
      return;                                                                 \
    view_message_loop_->PostTask(                                             \
        FROM_HERE,                                                            \
        base::Bind(&ViewDelegate::name, base::Unretained(view_delegate_), p1, \
                   p2, p3, p4, p5));                                          \
  }

DEFINE_VIEW_DELEGATE_2(AddWindow, WindowId, WindowId)
DEFINE_VIEW_DELEGATE_2(ChangeParentWindow, WindowId, WindowId)
DEFINE_VIEW_DELEGATE_1(CreateEditorWindow, const EditorWindow*)
DEFINE_VIEW_DELEGATE_3(CreateFormWindow,
                       WindowId,
                       Form*,
                       const domapi::PopupWindowInit&)
DEFINE_VIEW_DELEGATE_2(CreateTableWindow, WindowId, Document*)
DEFINE_VIEW_DELEGATE_2(CreateTextWindow, WindowId, text::Selection*)
DEFINE_VIEW_DELEGATE_1(DestroyWindow, WindowId)
DEFINE_VIEW_DELEGATE_1(DidStartScriptHost, domapi::ScriptHostState)
DEFINE_VIEW_DELEGATE_1(FocusWindow, WindowId)
DEFINE_VIEW_DELEGATE_3(GetFileNameForLoad,
                       WindowId,
                       const base::string16&,
                       const GetFileNameForLoadResolver&)
DEFINE_VIEW_DELEGATE_3(GetFileNameForSave,
                       WindowId,
                       const base::string16&,
                       const GetFileNameForSaveResolver&)
DEFINE_VIEW_DELEGATE_1(HideWindow, WindowId)
DEFINE_VIEW_DELEGATE_1(MakeSelectionVisible, WindowId)
DEFINE_VIEW_DELEGATE_5(MessageBox,
                       WindowId,
                       const base::string16&,
                       const ::base::string16&,
                       int,
                       const MessageBoxResolver&)
DEFINE_VIEW_DELEGATE_2(Reconvert, WindowId, const base::string16&);
DEFINE_VIEW_DELEGATE_1(RealizeWindow, WindowId)
DEFINE_VIEW_DELEGATE_1(ReleaseCapture, domapi::EventTargetId)
DEFINE_VIEW_DELEGATE_1(SetCapture, domapi::EventTargetId)
DEFINE_VIEW_DELEGATE_2(SetStatusBar,
                       WindowId,
                       const std::vector<base::string16>&)
DEFINE_VIEW_DELEGATE_2(SetTabData, WindowId, const domapi::TabData&)
DEFINE_VIEW_DELEGATE_2(SetTextWindowZoom, WindowId, float)
DEFINE_VIEW_DELEGATE_1(ShowWindow, WindowId)
DEFINE_VIEW_DELEGATE_2(SplitHorizontally, WindowId, WindowId)
DEFINE_VIEW_DELEGATE_2(SplitVertically, WindowId, WindowId)

#define DEFINE_SYNC_VIEW_DELEGATE_0(name, return_type)                     \
  return_type ScriptThread::name() {                                       \
    DCHECK_CALLED_ON_SCRIPT_THREAD();                                      \
    if (!view_message_loop_)                                               \
      return return_type();                                                \
    return DoSynchronousCall(                                              \
        base::Bind(&ViewDelegate::name, base::Unretained(view_delegate_)), \
        view_message_loop_, waitable_event_.get());                        \
  }

#define DEFINE_SYNC_VIEW_DELEGATE_1(name, return_type, type1)                  \
  return_type ScriptThread::name(type1 p1) {                                   \
    DCHECK_CALLED_ON_SCRIPT_THREAD();                                          \
    if (!view_message_loop_)                                                   \
      return return_type();                                                    \
    return DoSynchronousCall(                                                  \
        base::Bind(&ViewDelegate::name, base::Unretained(view_delegate_), p1), \
        view_message_loop_, waitable_event_.get());                            \
  }

#define DEFINE_SYNC_VIEW_DELEGATE_2(name, return_type, type1, type2)          \
  return_type ScriptThread::name(type1 p1, type2 p2) {                        \
    DCHECK_CALLED_ON_SCRIPT_THREAD();                                         \
    if (!view_message_loop_)                                                  \
      return return_type();                                                   \
    return DoSynchronousCall(                                                 \
        base::Bind(&ViewDelegate::name, base::Unretained(view_delegate_), p1, \
                   p2),                                                       \
        view_message_loop_, waitable_event_.get());                           \
  }

#define DEFINE_SYNC_VIEW_DELEGATE_3(name, return_type, type1, type2, type3)   \
  return_type ScriptThread::name(type1 p1, type2 p2, type3 p3) {              \
    DCHECK_CALLED_ON_SCRIPT_THREAD();                                         \
    if (!view_message_loop_)                                                  \
      return return_type();                                                   \
    return DoSynchronousCall(                                                 \
        base::Bind(&ViewDelegate::name, base::Unretained(view_delegate_), p1, \
                   p2, p3),                                                   \
        view_message_loop_, waitable_event_.get());                           \
  }

DEFINE_SYNC_VIEW_DELEGATE_2(ComputeOnTextWindow,
                            text::Posn,
                            WindowId,
                            const TextWindowCompute&);
DEFINE_SYNC_VIEW_DELEGATE_1(GetMetrics, base::string16, const base::string16&)
DEFINE_SYNC_VIEW_DELEGATE_1(GetSwitch,
                            domapi::SwitchValue,
                            const base::string16&)
DEFINE_SYNC_VIEW_DELEGATE_0(GetSwitchNames, std::vector<base::string16>)
DEFINE_SYNC_VIEW_DELEGATE_2(GetTableRowStates,
                            std::vector<int>,
                            WindowId,
                            const std::vector<base::string16>&)
DEFINE_SYNC_VIEW_DELEGATE_2(HitTestTextPosition,
                            domapi::FloatRect,
                            WindowId,
                            text::Posn)
DEFINE_SYNC_VIEW_DELEGATE_3(MapPointToPosition,
                            text::Posn,
                            domapi::EventTargetId,
                            float,
                            float)

void ScriptThread::ScrollTextWindow(WindowId window_id, int direction) {
  DCHECK_CALLED_ON_SCRIPT_THREAD();
  if (!view_message_loop_)
    return;
  RunSynchronously(
      base::Bind(&ViewDelegate::ScrollTextWindow,
                 base::Unretained(view_delegate_), window_id, direction),
      view_message_loop_, waitable_event_.get());
}

void ScriptThread::SetSwitch(const base::string16& name,
                             const domapi::SwitchValue& new_value) {
  DCHECK_CALLED_ON_SCRIPT_THREAD();
  if (!view_message_loop_)
    return;
  RunSynchronously(
      base::Bind(&ViewDelegate::SetSwitch, base::Unretained(view_delegate_),
                 name, new_value),
      view_message_loop_, waitable_event_.get());
}

void ScriptThread::UpdateWindow(WindowId window_id) {
  DCHECK_CALLED_ON_SCRIPT_THREAD();
  if (!view_message_loop_)
    return;
  RunSynchronously(base::Bind(&ViewDelegate::UpdateWindow,
                              base::Unretained(view_delegate_), window_id),
                   view_message_loop_, waitable_event_.get());
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
    thread_->message_loop()->PostTask(                               \
        FROM_HERE,                                                   \
        base::Bind(&ViewEventHandler::name,                          \
                   base::Unretained(view_event_handler()), param1)); \
  }

#define DEFINE_VIEW_EVENT_HANDLER2(name, type1, type2)                       \
  void ScriptThread::name(type1 param1, type2 param2) {                      \
    DCHECK_CALLED_ON_NON_SCRIPT_THREAD();                                    \
    thread_->message_loop()->PostTask(                                       \
        FROM_HERE,                                                           \
        base::Bind(&ViewEventHandler::name,                                  \
                   base::Unretained(view_event_handler()), param1, param2)); \
  }

#define DEFINE_VIEW_EVENT_HANDLER3(name, type1, type2, type3)                 \
  void ScriptThread::name(type1 param1, type2 param2, type3 param3) {         \
    DCHECK_CALLED_ON_NON_SCRIPT_THREAD();                                     \
    thread_->message_loop()->PostTask(                                        \
        FROM_HERE, base::Bind(&ViewEventHandler::name,                        \
                              base::Unretained(view_event_handler()), param1, \
                              param2, param3));                               \
  }

#define DEFINE_VIEW_EVENT_HANDLER4(name, type1, type2, type3, type4)          \
  void ScriptThread::name(type1 param1, type2 param2, type3 param3,           \
                          type4 param4) {                                     \
    DCHECK_CALLED_ON_NON_SCRIPT_THREAD();                                     \
    thread_->message_loop()->PostTask(                                        \
        FROM_HERE, base::Bind(&ViewEventHandler::name,                        \
                              base::Unretained(view_event_handler()), param1, \
                              param2, param3, param4));                       \
  }

#define DEFINE_VIEW_EVENT_HANDLER5(name, type1, type2, type3, type4, type5)   \
  void ScriptThread::name(type1 param1, type2 param2, type3 param3,           \
                          type4 param4, type5 param5) {                       \
    DCHECK_CALLED_ON_NON_SCRIPT_THREAD();                                     \
    thread_->message_loop()->PostTask(                                        \
        FROM_HERE, base::Bind(&ViewEventHandler::name,                        \
                              base::Unretained(view_event_handler()), param1, \
                              param2, param3, param4, param5));               \
  }

DEFINE_VIEW_EVENT_HANDLER1(DidBeginFrame, const base::Time&)
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
    auto const isolate = script_host_->isolate();
    isolate->TerminateExecution();
    return;
  }

  thread_->message_loop()->PostTask(
      FROM_HERE, base::Bind(&ViewEventHandler::DispatchKeyboardEvent,
                            base::Unretained(view_event_handler()), event));
}

DEFINE_VIEW_EVENT_HANDLER1(DispatchMouseEvent, const domapi::MouseEvent&)
DEFINE_VIEW_EVENT_HANDLER1(DispatchTextCompositionEvent,
                           const domapi::TextCompositionEvent&)
DEFINE_VIEW_EVENT_HANDLER1(DispatchWheelEvent, const domapi::WheelEvent&)
DEFINE_VIEW_EVENT_HANDLER2(OpenFile, WindowId, const base::string16&)
DEFINE_VIEW_EVENT_HANDLER2(ProcessCommandLine,
                           base::string16,
                           const std::vector<base::string16>&)
DEFINE_VIEW_EVENT_HANDLER1(QueryClose, WindowId)
DEFINE_VIEW_EVENT_HANDLER1(RunCallback, base::Closure)

void ScriptThread::WillDestroyViewHost() {
  DCHECK_CALLED_ON_NON_SCRIPT_THREAD();
  view_message_loop_ = nullptr;
  thread_->message_loop()->PostTask(
      FROM_HERE, base::Bind(&ViewEventHandler::WillDestroyViewHost,
                            base::Unretained(view_event_handler())));
}

}  // namespace dom
