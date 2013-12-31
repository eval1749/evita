// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/script_thread.h"

#pragma warning(push)
#pragma warning(disable: 4100 4625)
#include "base/message_loop/message_loop.h"
#pragma warning(pop)
#pragma warning(push)
#pragma warning(disable: 4625)
#include "base/threading/thread.h"
#pragma warning(pop)
#include "evita/dom/lock.h"
#include "evita/dom/script_controller.h"

#define DCHECK_CALLED_ON_HOST_THREAD() \
  DCHECK(thread_checker_.CalledOnValidThread());

#define DCHECK_CALLED_ON_SCRIPT_THREAD() \
  DCHECK(ScriptThread::instance()->CalledOnValidThread())

namespace dom {

namespace {
ScriptThread* script_thread;

void StartScriptController(ViewDelegate* view_delegate) {
  ScriptController::Start(view_delegate);
}
}  // namespace

ScriptThread::ScriptThread(ViewDelegate* view_delegate,
                           base::MessageLoop* host_message_loop)
    : view_delegate_(view_delegate),
      host_message_loop_(host_message_loop),
      thread_(new base::Thread("script_thread")),
      view_event_handler_(nullptr) {
  thread_->Start();
  PostTask(FROM_HERE, base::Bind(StartScriptController,
                                 base::Unretained(this)));
}

ScriptThread::~ScriptThread() {
  script_thread = nullptr;
}

ScriptThread* ScriptThread::instance() {
  DCHECK(script_thread);
  return script_thread;
}

bool ScriptThread::CalledOnValidThread() const {
  return CalledOnScriptThread();
}

bool ScriptThread::CalledOnScriptThread() const {
  return base::MessageLoop::current() == thread_->message_loop();
}

void ScriptThread::Evaluate(
    const base::string16& script_text,
    base::Callback<void(EvaluateResult)> callback) {
  DCHECK_CALLED_ON_HOST_THREAD();
  PostTask(FROM_HERE, base::Bind(&ScriptThread::EvaluateImpl,
      base::Unretained(this), script_text, callback));
}

void ScriptThread::EvaluateImpl(
    base::string16 script_text,
    base::Callback<void(EvaluateResult)> callback) {
  DCHECK_CALLED_ON_SCRIPT_THREAD();
  if (!host_message_loop_) {
    return;
  }

  DOM_AUTO_LOCK_SCOPE();
  auto eval_result = ScriptController::instance()->Evaluate(script_text);
  host_message_loop_->PostTask(FROM_HERE, base::Bind(callback, eval_result));
}

void ScriptThread::PostTask(const tracked_objects::Location& from_here,
                            const base::Closure& task) {
  DCHECK_CALLED_ON_HOST_THREAD();
  thread_->message_loop()->PostTask(from_here, task);
}

// ViewDelegate
void ScriptThread::CreateEditorWindow(const EditorWindow* window) {
  DCHECK_CALLED_ON_SCRIPT_THREAD();
  if (!host_message_loop_)
    return;
  host_message_loop_->PostTask(FROM_HERE, base::Bind(
      &ViewDelegate::CreateEditorWindow,
      base::Unretained(view_delegate_),
      window));
}

void ScriptThread::CreateTextWindow(const TextWindow* window) {
  DCHECK_CALLED_ON_SCRIPT_THREAD();
  if (!host_message_loop_)
    return;
  host_message_loop_->PostTask(FROM_HERE, base::Bind(
      &ViewDelegate::CreateTextWindow,
      base::Unretained(view_delegate_),
      window));
}

void ScriptThread::DestroyWindow(WidgetId widget_id) {
  DCHECK_CALLED_ON_SCRIPT_THREAD();
  if (!host_message_loop_)
    return;
  host_message_loop_->PostTask(FROM_HERE, base::Bind(
      &ViewDelegate::DestroyWindow,
      base::Unretained(view_delegate_),
      widget_id));
}

void ScriptThread::RealizeWindow(WidgetId widget_id) {
  DCHECK_CALLED_ON_SCRIPT_THREAD();
  if (!host_message_loop_)
    return;
  host_message_loop_->PostTask(FROM_HERE, base::Bind(
      &ViewDelegate::RealizeWindow,
      base::Unretained(view_delegate_),
      widget_id));
}

void ScriptThread::RegisterViewEventHandler(
    ViewEventHandler* event_handler) {
  DCHECK_CALLED_ON_SCRIPT_THREAD();
  if (!host_message_loop_)
    return;
  view_event_handler_ = event_handler;
  host_message_loop_->PostTask(FROM_HERE, base::Bind(
      &ViewDelegate::RegisterViewEventHandler,
      base::Unretained(view_delegate_),
      base::Unretained(this)));
}

void ScriptThread::Start(ViewDelegate* view_delegate,
                         base::MessageLoop* host_message_loop) {
  DCHECK(!script_thread);
  script_thread = new ScriptThread(view_delegate, host_message_loop);
}

// ViewEventHandler
void ScriptThread::DidDestroyWidget(WidgetId widget_id) {
  DCHECK_CALLED_ON_HOST_THREAD();
  DCHECK(view_event_handler_);
  PostTask(FROM_HERE, base::Bind(
      &ViewEventHandler::DidDestroyWidget,
      base::Unretained(view_event_handler_),
      widget_id));
}

void ScriptThread::WillDestroyHost() {
  DCHECK_CALLED_ON_HOST_THREAD();
  DCHECK(view_event_handler_);
  view_delegate_ = nullptr;
  host_message_loop_ = nullptr;
  PostTask(FROM_HERE, base::Bind(
      &ViewEventHandler::WillDestroyHost,
      base::Unretained(view_event_handler_)));
}

}  // namespace dom
