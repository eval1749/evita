// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/script_thread.h"

#pragma warning(push)
#pragma warning(disable: 4100 4625 4626)
#include "base/bind.h"
#include "base/callback.h"
#include "base/message_loop/message_loop.h"
#include "base/synchronization/waitable_event.h"
#include "base/threading/thread.h"
#pragma warning(pop)
#include "evita/dom/lock.h"
#include "evita/dom/public/float_point.h"
#include "evita/dom/public/tab_data.h"
#include "evita/dom/public/view_event.h"
#include "evita/dom/script_host.h"
#include "evita/ui/events/event.h"
#include "v8/include/v8-debug.h"

#define DCHECK_CALLED_ON_NON_SCRIPT_THREAD() \
  DCHECK_NE(script_thread->message_loop(), base::MessageLoop::current())

#define DCHECK_CALLED_ON_SCRIPT_THREAD() \
  DCHECK_EQ(script_thread->message_loop(), base::MessageLoop::current())

namespace dom {

namespace {

base::Thread* script_thread;

//////////////////////////////////////////////////////////////////////
//
// SynchronousCaller
// Run task on another thread and wait for task done.
//
template<typename Result, typename... Params>
class SynchronousCaller {
  private: base::Callback<Result(Params...)> task_;
  private: base::WaitableEvent* event_;
  private: Result result_;

  public: SynchronousCaller(const base::Callback<Result(Params...)>& task,
                            base::WaitableEvent* event)
    : event_(event), task_(task) {
  }

  public: ~SynchronousCaller() = default;

  public: Result Call(base::MessageLoop* message_loop) {
    DCHECK_CALLED_ON_SCRIPT_THREAD();
    event_->Reset();
    DOM_AUTO_UNLOCK_SCOPE();
    message_loop->PostTask(FROM_HERE, base::Bind(
        &SynchronousCaller::RunTask, base::Unretained(this)));
    event_->Wait();
    return result_;
  }

  private: void RunTask() {
    DCHECK_CALLED_ON_NON_SCRIPT_THREAD();
    result_ = task_.Run();
    event_->Signal();
  }

  DISALLOW_COPY_AND_ASSIGN(SynchronousCaller);
};

template<typename Result, typename... Params>
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
template<typename... Params>
class SynchronousRunner {
  private: base::Callback<void(Params...)> task_;
  private: base::WaitableEvent* event_;

  public: SynchronousRunner(const base::Callback<void(Params...)>& task,
                            base::WaitableEvent* event)
    : event_(event), task_(task) {
  }

  public: ~SynchronousRunner() = default;

  public: void Run(base::MessageLoop* message_loop) {
    DCHECK_CALLED_ON_SCRIPT_THREAD();
    event_->Reset();
    DOM_AUTO_UNLOCK_SCOPE();
    message_loop->PostTask(FROM_HERE, base::Bind(
        &SynchronousRunner::RunTask, base::Unretained(this)));
    event_->Wait();
  }

  private: void RunTask() {
    DCHECK_CALLED_ON_NON_SCRIPT_THREAD();
    task_.Run();
    event_->Signal();
  }

  DISALLOW_COPY_AND_ASSIGN(SynchronousRunner);
};

template<typename... Params>
void RunSynchronously(const base::Callback<void(Params...)>& task,
                      base::MessageLoop* message_loop,
                      base::WaitableEvent* event) {
  SynchronousRunner<Params...> caller(task, event);
  caller.Run(message_loop);
}

void PostScriptTask(const tracked_objects::Location& from_here,
                            const base::Closure& task) {
  DCHECK_CALLED_ON_NON_SCRIPT_THREAD();
  script_thread->message_loop()->PostTask(from_here, task);
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// ScriptThread
//
ScriptThread::ScriptThread(base::MessageLoop* view_message_loop,
                           ViewDelegate* view_delegate,
                           base::MessageLoop* io_message_loop,
                           domapi::IoDelegate* io_delegate)
    : io_delegate_(io_delegate),
      io_message_loop_(io_message_loop),
      view_delegate_(view_delegate),
      view_event_handler_(nullptr),
      view_message_loop_(view_message_loop),
      waitable_event_(new base::WaitableEvent(true, false)) {
  DCHECK(!script_thread);
  script_thread = new base::Thread("script_thread");
  script_thread->Start();
}

ScriptThread::~ScriptThread() {
  script_thread = nullptr;
}

void ScriptThread::Start(base::MessageLoop* view_message_loop,
                         ViewDelegate* view_delegate,
                         base::MessageLoop* io_message_loop,
                         domapi::IoDelegate* io_delegate) {
  auto const script_thread = new ScriptThread(view_message_loop, view_delegate,
                                              io_message_loop, io_delegate);
  PostScriptTask(FROM_HERE, base::Bind(
      base::IgnoreResult(&ScriptHost::Start),
      base::Unretained(script_thread),
      base::Unretained(script_thread)));
}

// IoDelegate
#define DEFINE_IO_DELEGATE_2(name, type1, type2) \
  void ScriptThread::name(type1 p1, type2 p2) { \
    io_message_loop_->PostTask(FROM_HERE, base::Bind( \
        &IoDelegate::name, base::Unretained(io_delegate_), p1, p2)); \
  }

#define DEFINE_IO_DELEGATE_3(name, type1, type2, type3) \
  void ScriptThread::name(type1 p1, type2 p2, type3 p3) { \
    io_message_loop_->PostTask(FROM_HERE, base::Bind( \
        &IoDelegate::name, base::Unretained(io_delegate_), p1, p2, p3)); \
  }

#define DEFINE_IO_DELEGATE_4(name, type1, type2, type3, type4) \
  void ScriptThread::name(type1 p1, type2 p2, type3 p3, type4 p4) { \
    io_message_loop_->PostTask(FROM_HERE, base::Bind( \
        &IoDelegate::name, base::Unretained(io_delegate_), p1, p2, p3, p4)); \
  }

DEFINE_IO_DELEGATE_2(CloseFile, domapi::IoContextId,
                     const domapi::FileIoDeferred&)
DEFINE_IO_DELEGATE_3(MakeTempFileName, const base::string16&,
                     const base::string16&,
                     const domapi::MakeTempFileNameResolver&)
DEFINE_IO_DELEGATE_4(MoveFile, const base::string16&, const base::string16&,
                     const domapi::MoveFileOptions&,
                     const domapi::IoResolver&)
DEFINE_IO_DELEGATE_3(OpenFile, const base::string16&,
                     const base::string16&,
                     const domapi::OpenFileDeferred&)
DEFINE_IO_DELEGATE_2(OpenProcess, const base::string16&,
                     const domapi::OpenProcessDeferred&)
DEFINE_IO_DELEGATE_2(QueryFileStatus, const base::string16&,
                     const domapi::QueryFileStatusDeferred&)
DEFINE_IO_DELEGATE_4(ReadFile, domapi::IoContextId, void*, size_t,
                     const domapi::FileIoDeferred&)
DEFINE_IO_DELEGATE_2(RemoveFile, const base::string16&,
                     const domapi::IoResolver&)
DEFINE_IO_DELEGATE_4(WriteFile, domapi::IoContextId, void*, size_t,
                     const domapi::FileIoDeferred&)

// ViewDelegate
#define DEFINE_VIEW_DELEGATE_1(name, type1) \
  void ScriptThread::name(type1 param1) { \
    DCHECK_CALLED_ON_SCRIPT_THREAD(); \
    if (!view_message_loop_) \
      return; \
    view_message_loop_->PostTask(FROM_HERE, base::Bind( \
        &ViewDelegate::name, \
        base::Unretained(view_delegate_), \
        param1)); \
  }

#define DEFINE_VIEW_DELEGATE_2(name, type1, type2) \
  void ScriptThread::name(type1 param1, type2 param2) { \
    DCHECK_CALLED_ON_SCRIPT_THREAD(); \
    if (!view_message_loop_) \
      return; \
    view_message_loop_->PostTask(FROM_HERE, base::Bind( \
        &ViewDelegate::name, \
        base::Unretained(view_delegate_), \
        param1, param2)); \
  }

#define DEFINE_VIEW_DELEGATE_3(name, type1, type2, type3) \
  void ScriptThread::name(type1 param1, type2 param2, type3 param3) { \
    DCHECK_CALLED_ON_SCRIPT_THREAD(); \
    if (!view_message_loop_) \
      return; \
    view_message_loop_->PostTask(FROM_HERE, base::Bind( \
        &ViewDelegate::name, \
        base::Unretained(view_delegate_), \
        param1, param2, param3)); \
  }

#define DEFINE_VIEW_DELEGATE_4(name, type1, type2, type3, type4) \
  void ScriptThread::name(type1 p1, type2 p2, type3 p3, type4 p4) { \
    DCHECK_CALLED_ON_SCRIPT_THREAD(); \
    if (!view_message_loop_) \
      return; \
    view_message_loop_->PostTask(FROM_HERE, base::Bind( \
        &ViewDelegate::name, \
        base::Unretained(view_delegate_), \
        p1, p2, p3, p4)); \
  }

#define DEFINE_VIEW_DELEGATE_5(name, type1, type2, type3, type4, type5) \
  void ScriptThread::name(type1 p1, type2 p2, type3 p3, type4 p4, type5 p5) { \
    DCHECK_CALLED_ON_SCRIPT_THREAD(); \
    if (!view_message_loop_) \
      return; \
    view_message_loop_->PostTask(FROM_HERE, base::Bind( \
        &ViewDelegate::name, \
        base::Unretained(view_delegate_), \
        p1, p2, p3, p4, p5)); \
  }

DEFINE_VIEW_DELEGATE_2(AddWindow, WindowId, WindowId)
DEFINE_VIEW_DELEGATE_2(ChangeParentWindow, WindowId, WindowId)
DEFINE_VIEW_DELEGATE_2(CheckSpelling, const base::string16&,
    const CheckSpellingDeferred&)
DEFINE_VIEW_DELEGATE_1(CreateEditorWindow, const EditorWindow*)
DEFINE_VIEW_DELEGATE_2(CreateFormWindow, WindowId, const Form*)
DEFINE_VIEW_DELEGATE_2(CreateTableWindow, WindowId, Document*)
DEFINE_VIEW_DELEGATE_1(CreateTextWindow, const TextWindow*)
DEFINE_VIEW_DELEGATE_1(DestroyWindow, WindowId)
DEFINE_VIEW_DELEGATE_1(DidStartScriptHost, domapi::ScriptHostState)
DEFINE_VIEW_DELEGATE_1(DidHandleViewIdelEvent, int);
DEFINE_VIEW_DELEGATE_1(FocusWindow, WindowId)
DEFINE_VIEW_DELEGATE_3(GetFilenameForLoad, WindowId, const base::string16&,
                       ViewDelegate::GetFilenameForLoadCallback)
DEFINE_VIEW_DELEGATE_3(GetFilenameForSave, WindowId, const base::string16&,
                       ViewDelegate::GetFilenameForSaveCallback)
DEFINE_VIEW_DELEGATE_2(GetSpellingSuggestions, const base::string16&,
    const GetSpellingSuggestionsDeferred&)
DEFINE_VIEW_DELEGATE_1(HideWindow, WindowId)
DEFINE_VIEW_DELEGATE_1(MakeSelectionVisible, WindowId)
DEFINE_VIEW_DELEGATE_5(MessageBox, WindowId, const base::string16&,
                       const::base::string16&, int,
                       MessageBoxCallback)
DEFINE_VIEW_DELEGATE_3(Reconvert, WindowId, text::Posn, text::Posn);
DEFINE_VIEW_DELEGATE_1(RealizeWindow, WindowId)
DEFINE_VIEW_DELEGATE_1(ReleaseCapture, WindowId)
DEFINE_VIEW_DELEGATE_1(SetCapture, WindowId)
DEFINE_VIEW_DELEGATE_2(SetStatusBar, WindowId,
    const std::vector<base::string16>&)
DEFINE_VIEW_DELEGATE_2(SetTabData, WindowId, const domapi::TabData&)
DEFINE_VIEW_DELEGATE_1(ShowWindow, WindowId)
DEFINE_VIEW_DELEGATE_2(SplitHorizontally, WindowId, WindowId)
DEFINE_VIEW_DELEGATE_2(SplitVertically, WindowId, WindowId)

#define DEFINE_SYNC_VIEW_DELEGATE_1(name, return_type, type1) \
  return_type ScriptThread::name(type1 p1) { \
    DCHECK_CALLED_ON_SCRIPT_THREAD(); \
    if (!view_message_loop_) \
      return return_type(); \
    return DoSynchronousCall( \
          base::Bind(&ViewDelegate::name, \
                     base::Unretained(view_delegate_), p1), \
          view_message_loop_, waitable_event_.get()); \
  }

#define DEFINE_SYNC_VIEW_DELEGATE_2(name, return_type, type1, type2) \
  return_type ScriptThread::name(type1 p1, type2 p2) { \
    DCHECK_CALLED_ON_SCRIPT_THREAD(); \
    if (!view_message_loop_) \
      return return_type(); \
    return DoSynchronousCall( \
          base::Bind(&ViewDelegate::name, \
                     base::Unretained(view_delegate_), p1, p2), \
          view_message_loop_, waitable_event_.get()); \
  }

#define DEFINE_SYNC_VIEW_DELEGATE_3(name, return_type, type1, type2, type3) \
  return_type ScriptThread::name(type1 p1, type2 p2, type3 p3) { \
    DCHECK_CALLED_ON_SCRIPT_THREAD(); \
    if (!view_message_loop_) \
      return return_type(); \
    return DoSynchronousCall( \
          base::Bind(&ViewDelegate::name, \
                     base::Unretained(view_delegate_), p1, p2, p3), \
          view_message_loop_, waitable_event_.get()); \
  }

DEFINE_SYNC_VIEW_DELEGATE_2(ComputeOnTextWindow, text::Posn,
                            WindowId, const TextWindowCompute&);
DEFINE_SYNC_VIEW_DELEGATE_1(GetMetrics, base::string16, const base::string16&)
DEFINE_SYNC_VIEW_DELEGATE_2(GetTableRowStates, std::vector<int>, WindowId,
                            const std::vector<base::string16>&)
DEFINE_SYNC_VIEW_DELEGATE_3(MapPointToPosition, text::Posn,
                            domapi::EventTargetId, float, float)
DEFINE_SYNC_VIEW_DELEGATE_2(MapPositionToPoint, domapi::FloatPoint,
                            WindowId, text::Posn)


void ScriptThread::RegisterViewEventHandler(
    domapi::ViewEventHandler* event_handler) {
  DCHECK_CALLED_ON_SCRIPT_THREAD();
  if (!view_message_loop_)
    return;
  view_event_handler_ = event_handler;
  view_message_loop_->PostTask(FROM_HERE, base::Bind(
      &ViewDelegate::RegisterViewEventHandler,
      base::Unretained(view_delegate_),
      base::Unretained(this)));
}

void ScriptThread::ScrollTextWindow(WindowId window_id, int direction) {
  DCHECK_CALLED_ON_SCRIPT_THREAD();
  if (!view_message_loop_)
    return;
  RunSynchronously(base::Bind(
      &ViewDelegate::ScrollTextWindow, base::Unretained(view_delegate_),
      window_id, direction), view_message_loop_, waitable_event_.get());
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
#define DEFINE_VIEW_EVENT_HANDLER_0(name) \
  void ScriptThread::name() { \
    DCHECK_CALLED_ON_NON_SCRIPT_THREAD(); \
    DCHECK(view_event_handler_); \
    PostScriptTask(FROM_HERE, base::Bind( \
        &ViewEventHandler::name, \
        base::Unretained(view_event_handler_))); \
  }

#define DEFINE_VIEW_EVENT_HANDLER_1(name, type1) \
  void ScriptThread::name(type1 param1) { \
    DCHECK_CALLED_ON_NON_SCRIPT_THREAD(); \
    DCHECK(view_event_handler_); \
    PostScriptTask(FROM_HERE, base::Bind( \
        &ViewEventHandler::name, \
        base::Unretained(view_event_handler_), \
        param1)); \
  }

#define DEFINE_VIEW_EVENT_HANDLER_2(name, type1, type2) \
  void ScriptThread::name(type1 param1, type2 param2) { \
    DCHECK_CALLED_ON_NON_SCRIPT_THREAD(); \
    DCHECK(view_event_handler_); \
    PostScriptTask(FROM_HERE, base::Bind( \
        &ViewEventHandler::name, \
        base::Unretained(view_event_handler_), \
        param1, param2)); \
  }

#define DEFINE_VIEW_EVENT_HANDLER_3(name, type1, type2, type3) \
  void ScriptThread::name(type1 param1, type2 param2, type3 param3) { \
    DCHECK_CALLED_ON_NON_SCRIPT_THREAD(); \
    DCHECK(view_event_handler_); \
    PostScriptTask(FROM_HERE, base::Bind( \
        &ViewEventHandler::name, \
        base::Unretained(view_event_handler_), \
        param1, param2, param3)); \
  }

#define DEFINE_VIEW_EVENT_HANDLER_4(name, type1, type2, type3, type4) \
  void ScriptThread::name(type1 param1, type2 param2, type3 param3, \
                          type4 param4) { \
    DCHECK_CALLED_ON_NON_SCRIPT_THREAD(); \
    DCHECK(view_event_handler_); \
    PostScriptTask(FROM_HERE, base::Bind( \
        &ViewEventHandler::name, \
        base::Unretained(view_event_handler_), \
        param1, param2, param3, param4)); \
  }

#define DEFINE_VIEW_EVENT_HANDLER_5(name, type1, type2, type3, type4, type5) \
  void ScriptThread::name(type1 param1, type2 param2, type3 param3, \
                          type4 param4, type5 param5) { \
    DCHECK_CALLED_ON_NON_SCRIPT_THREAD(); \
    DCHECK(view_event_handler_); \
    PostScriptTask(FROM_HERE, base::Bind( \
        &ViewEventHandler::name, \
        base::Unretained(view_event_handler_), \
        param1, param2, param3, param4, param5)); \
  }

DEFINE_VIEW_EVENT_HANDLER_2(AppendTextToBuffer, text::Buffer*,
                            const base::string16&)
DEFINE_VIEW_EVENT_HANDLER_1(DidDestroyWidget, WindowId)
DEFINE_VIEW_EVENT_HANDLER_2(DidDropWidget, WindowId, WindowId)
DEFINE_VIEW_EVENT_HANDLER_1(DidRealizeWidget, WindowId)
DEFINE_VIEW_EVENT_HANDLER_5(DidResizeWidget, WindowId, int, int, int, int)
DEFINE_VIEW_EVENT_HANDLER_0(DidStartViewHost)
DEFINE_VIEW_EVENT_HANDLER_1(DispatchFocusEvent, const domapi::FocusEvent&)

void ScriptThread::DispatchKeyboardEvent(const domapi::KeyboardEvent& event) {
  DCHECK_CALLED_ON_NON_SCRIPT_THREAD();
  DCHECK(view_event_handler_);
  if (event.key_code == (static_cast<int>(ui::KeyCode::Pause) |
                         static_cast<int>(ui::Modifier::Control)) &&
      event.control_key) {
    auto const isolate = ScriptHost::instance()->isolate();
    v8::V8::TerminateExecution(isolate);
    return;
  }

  PostScriptTask(FROM_HERE, base::Bind(&ViewEventHandler::DispatchKeyboardEvent,
           base::Unretained(view_event_handler_), event));
}

DEFINE_VIEW_EVENT_HANDLER_1(DispatchMouseEvent, const domapi::MouseEvent&)
DEFINE_VIEW_EVENT_HANDLER_1(DispatchViewIdleEvent, int)
DEFINE_VIEW_EVENT_HANDLER_1(DispatchWheelEvent,
    const domapi::WheelEvent&)
DEFINE_VIEW_EVENT_HANDLER_2(OpenFile, WindowId, const base::string16&)
DEFINE_VIEW_EVENT_HANDLER_1(QueryClose, WindowId)
DEFINE_VIEW_EVENT_HANDLER_1(RunCallback, base::Closure)

void ScriptThread::WillDestroyHost() {
  DCHECK_CALLED_ON_NON_SCRIPT_THREAD();
  DCHECK(view_event_handler_);
  view_delegate_ = nullptr;
  view_message_loop_ = nullptr;
  PostScriptTask(FROM_HERE, base::Bind(
      &ViewEventHandler::WillDestroyHost,
      base::Unretained(view_event_handler_)));
}

}  // namespace dom
