// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include <vector>

#include "evita/views/view_thread_proxy.h"

#include "base/bind.h"
#include "base/callback.h"
#include "base/message_loop/message_loop.h"
#include "base/synchronization/waitable_event.h"
#include "base/trace_event/trace_event.h"
#include "evita/dom/lock.h"
#include "evita/dom/public/float_rect.h"
#include "evita/dom/public/form.h"
#include "evita/dom/public/tab_data.h"
#include "evita/dom/public/text_area_display_item.h"
#include "evita/views/view_delegate_impl.h"
#include "evita/visuals/display/public/display_item_list.h"

namespace views {

#define DCHECK_CALLED_ON_SCRIPT_THREAD() \
  DCHECK_NE(message_loop_, base::MessageLoop::current())

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
// ViewThreadProxy
//
ViewThreadProxy::ViewThreadProxy(base::MessageLoop* message_loop)
    : delegate_(new ViewDelegateImpl()),
      message_loop_(message_loop),
      waitable_event_(new base::WaitableEvent(
          base::WaitableEvent::ResetPolicy::MANUAL,
          base::WaitableEvent::InitialState::NOT_SIGNALED)) {}

ViewThreadProxy::~ViewThreadProxy() {}

// ViewDelegate
#define DEFINE_DELEGATE_0(name)                                              \
  void ViewThreadProxy::name() {                                             \
    DCHECK_CALLED_ON_SCRIPT_THREAD();                                        \
    if (!message_loop_)                                                      \
      return;                                                                \
    message_loop_->PostTask(                                                 \
        FROM_HERE,                                                           \
        base::Bind(&ViewDelegate::name, base::Unretained(delegate_.get()))); \
  }

#define DEFINE_DELEGATE_1(name, type1)                                     \
  void ViewThreadProxy::name(type1 param1) {                               \
    DCHECK_CALLED_ON_SCRIPT_THREAD();                                      \
    if (!message_loop_)                                                    \
      return;                                                              \
    message_loop_->PostTask(                                               \
        FROM_HERE, base::Bind(&ViewDelegate::name,                         \
                              base::Unretained(delegate_.get()), param1)); \
  }

#define DEFINE_DELEGATE_2(name, type1, type2)                              \
  void ViewThreadProxy::name(type1 param1, type2 param2) {                 \
    DCHECK_CALLED_ON_SCRIPT_THREAD();                                      \
    if (!message_loop_)                                                    \
      return;                                                              \
    message_loop_->PostTask(                                               \
        FROM_HERE,                                                         \
        base::Bind(&ViewDelegate::name, base::Unretained(delegate_.get()), \
                   param1, param2));                                       \
  }

#define DEFINE_DELEGATE_3(name, type1, type2, type3)                       \
  void ViewThreadProxy::name(type1 param1, type2 param2, type3 param3) {   \
    DCHECK_CALLED_ON_SCRIPT_THREAD();                                      \
    if (!message_loop_)                                                    \
      return;                                                              \
    message_loop_->PostTask(                                               \
        FROM_HERE,                                                         \
        base::Bind(&ViewDelegate::name, base::Unretained(delegate_.get()), \
                   param1, param2, param3));                               \
  }

#define DEFINE_DELEGATE_4(name, type1, type2, type3, type4)                    \
  void ViewThreadProxy::name(type1 p1, type2 p2, type3 p3, type4 p4) {         \
    DCHECK_CALLED_ON_SCRIPT_THREAD();                                          \
    if (!message_loop_)                                                        \
      return;                                                                  \
    message_loop_->PostTask(                                                   \
        FROM_HERE,                                                             \
        base::Bind(&ViewDelegate::name, base::Unretained(delegate_.get()), p1, \
                   p2, p3, p4));                                               \
  }

#define DEFINE_DELEGATE_5(name, type1, type2, type3, type4, type5)             \
  void ViewThreadProxy::name(type1 p1, type2 p2, type3 p3, type4 p4,           \
                             type5 p5) {                                       \
    DCHECK_CALLED_ON_SCRIPT_THREAD();                                          \
    if (!message_loop_)                                                        \
      return;                                                                  \
    message_loop_->PostTask(                                                   \
        FROM_HERE,                                                             \
        base::Bind(&ViewDelegate::name, base::Unretained(delegate_.get()), p1, \
                   p2, p3, p4, p5));                                           \
  }

DEFINE_DELEGATE_2(AddWindow, domapi::WindowId, domapi::WindowId)
DEFINE_DELEGATE_2(ChangeParentWindow, domapi::WindowId, domapi::WindowId)
DEFINE_DELEGATE_1(CreateEditorWindow, domapi::WindowId)
DEFINE_DELEGATE_4(CreateFormWindow,
                  domapi::WindowId,
                  domapi::WindowId,
                  const domapi::IntRect&,
                  const base::string16&)
DEFINE_DELEGATE_1(CreateTextWindow, domapi::WindowId)
DEFINE_DELEGATE_1(CreateVisualWindow, domapi::WindowId)
DEFINE_DELEGATE_1(DestroyWindow, domapi::WindowId)
DEFINE_DELEGATE_1(DidStartScriptHost, domapi::ScriptHostState)

DEFINE_DELEGATE_1(FocusWindow, domapi::WindowId)
DEFINE_DELEGATE_3(GetFileNameForLoad,
                  domapi::WindowId,
                  const base::string16&,
                  const GetFileNameForLoadResolver&)
DEFINE_DELEGATE_3(GetFileNameForSave,
                  domapi::WindowId,
                  const base::string16&,
                  const GetFileNameForSaveResolver&)
DEFINE_DELEGATE_2(GetMetrics,
                  const base::string16&,
                  const domapi::StringPromise&)
DEFINE_DELEGATE_1(HideWindow, domapi::WindowId)
DEFINE_DELEGATE_5(MessageBox,
                  domapi::WindowId,
                  const base::string16&,
                  const base::string16&,
                  int,
                  const MessageBoxResolver&)

void ViewThreadProxy::PaintForm(domapi::WindowId window_id,
                                std::unique_ptr<domapi::Form> form) {
  DCHECK_CALLED_ON_SCRIPT_THREAD();
  if (!message_loop_)
    return;
  message_loop_->PostTask(
      FROM_HERE,
      base::Bind(&ViewDelegate::PaintForm, base::Unretained(delegate_.get()),
                 window_id, base::Passed(std::move(form))));
}

void ViewThreadProxy::PaintTextArea(
    domapi::WindowId window_id,
    std::unique_ptr<domapi::TextAreaDisplayItem> display_item) {
  DCHECK_CALLED_ON_SCRIPT_THREAD();
  if (!message_loop_)
    return;
  message_loop_->PostTask(
      FROM_HERE, base::Bind(&ViewDelegate::PaintTextArea,
                            base::Unretained(delegate_.get()), window_id,
                            base::Passed(std::move(display_item))));
}

void ViewThreadProxy::PaintVisualDocument(
    domapi::WindowId window_id,
    std::unique_ptr<visuals::DisplayItemList> display_item_list) {
  DCHECK_CALLED_ON_SCRIPT_THREAD();
  if (!message_loop_)
    return;
  message_loop_->PostTask(
      FROM_HERE, base::Bind(&ViewDelegate::PaintVisualDocument,
                            base::Unretained(delegate_.get()), window_id,
                            base::Passed(std::move(display_item_list))));
}

DEFINE_DELEGATE_2(Reconvert, domapi::WindowId, const base::string16&);
DEFINE_DELEGATE_1(RealizeWindow, domapi::WindowId)
DEFINE_DELEGATE_1(ReleaseCapture, domapi::EventTargetId)
DEFINE_DELEGATE_1(SetCapture, domapi::EventTargetId)
DEFINE_DELEGATE_2(SetCursor, domapi::WindowId, domapi::CursorId)
DEFINE_DELEGATE_2(SetStatusBar,
                  domapi::WindowId,
                  const std::vector<base::string16>&)
DEFINE_DELEGATE_2(SetTabData, domapi::WindowId, const domapi::TabData&)
DEFINE_DELEGATE_1(ShowWindow, domapi::WindowId)
DEFINE_DELEGATE_2(SplitHorizontally, domapi::WindowId, domapi::WindowId)
DEFINE_DELEGATE_2(SplitVertically, domapi::WindowId, domapi::WindowId)
DEFINE_DELEGATE_1(StartTraceLog, const std::string&)
DEFINE_DELEGATE_1(StopTraceLog, const domapi::TraceLogOutputCallback&);

#define DEFINE_SYNC_DELEGATE_0(name, return_type)                           \
  return_type ViewThreadProxy::name() {                                     \
    DCHECK_CALLED_ON_SCRIPT_THREAD();                                       \
    if (!message_loop_)                                                     \
      return return_type();                                                 \
    TRACE_EVENT0("script", "ViewThreadProxy::" #name);                      \
    return DoSynchronousCall(                                               \
        base::Bind(&ViewDelegate::name, base::Unretained(delegate_.get())), \
        message_loop_, waitable_event_.get());                              \
  }

#define DEFINE_SYNC_DELEGATE_1(name, return_type, type1)                   \
  return_type ViewThreadProxy::name(type1 p1) {                            \
    DCHECK_CALLED_ON_SCRIPT_THREAD();                                      \
    if (!message_loop_)                                                    \
      return return_type();                                                \
    TRACE_EVENT0("script", "ViewThreadProxy::" #name);                     \
    return DoSynchronousCall(                                              \
        base::Bind(&ViewDelegate::name, base::Unretained(delegate_.get()), \
                   p1),                                                    \
        message_loop_, waitable_event_.get());                             \
  }

#define DEFINE_SYNC_DELEGATE_2(name, return_type, type1, type2)                \
  return_type ViewThreadProxy::name(type1 p1, type2 p2) {                      \
    DCHECK_CALLED_ON_SCRIPT_THREAD();                                          \
    if (!message_loop_)                                                        \
      return return_type();                                                    \
    TRACE_EVENT0("script", "ViewThreadProxy::" #name);                         \
    return DoSynchronousCall(                                                  \
        base::Bind(&ViewDelegate::name, base::Unretained(delegate_.get()), p1, \
                   p2),                                                        \
        message_loop_, waitable_event_.get());                                 \
  }

#define DEFINE_SYNC_DELEGATE_3(name, return_type, type1, type2, type3)         \
  return_type ViewThreadProxy::name(type1 p1, type2 p2, type3 p3) {            \
    DCHECK_CALLED_ON_SCRIPT_THREAD();                                          \
    if (!message_loop_)                                                        \
      return return_type();                                                    \
    TRACE_EVENT0("script", "ViewThreadProxy::" #name);                         \
    return DoSynchronousCall(                                                  \
        base::Bind(&ViewDelegate::name, base::Unretained(delegate_.get()), p1, \
                   p2, p3),                                                    \
        message_loop_, waitable_event_.get());                                 \
  }

DEFINE_SYNC_DELEGATE_1(GetSwitch, domapi::SwitchValue, const base::string16&)
DEFINE_SYNC_DELEGATE_0(GetSwitchNames, std::vector<base::string16>)

void ViewThreadProxy::SetSwitch(const base::string16& name,
                                const domapi::SwitchValue& new_value) {
  DCHECK_CALLED_ON_SCRIPT_THREAD();
  if (!message_loop_)
    return;
  TRACE_EVENT0("script", "ViewThreadProxy::SetSwitch");
  RunSynchronously(
      base::Bind(&ViewDelegate::SetSwitch, base::Unretained(delegate_.get()),
                 name, new_value),
      message_loop_, waitable_event_.get());
}

}  // namespace views
