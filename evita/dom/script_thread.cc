// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/script_thread.h"

#pragma warning(push)
#pragma warning(disable: 4100 4625)
#include "base/message_loop/message_loop.h"
#pragma warning(pop)
#include "base/threading/thread.h"
#include "evita/dom/script_controller.h"

namespace dom {

namespace {

void StartScriptController() {
  ScriptController::instance();
}

}  // namespace

ScriptThread::ScriptThread()
    : thread_(new base::Thread("script_thread")) {
  thread_->Start();
  PostTask(FROM_HERE, base::Bind(StartScriptController));
}

ScriptThread::~ScriptThread() {
}

bool ScriptThread::CalledOnValidThread() const {
  return base::MessageLoop::current() == thread_->message_loop();
}

void ScriptThread::PostTask(const tracked_objects::Location& from_here,
                            const base::Closure& task) {
  thread_->message_loop()->PostTask(from_here, task);
}

}  // namespace dom
