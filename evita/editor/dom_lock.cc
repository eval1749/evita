// Copyright (c) 1996-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/editor/dom_lock.h"

#include "base/memory/singleton.h"
#include "base/trace_event/trace_event.h"
#include "evita/dom/lock.h"
#include "evita/editor/application.h"

namespace editor {

//////////////////////////////////////////////////////////////////////
//
// DomLock::AutoLock
//
DomLock::AutoLock::AutoLock(const Location& location) {
  TRACE_EVENT_ASYNC_BEGIN2("view", "ViewLock", this, "function",
                           location.function_name(), "type", "AutoLock");
  if (DomLock::GetInstance()->TryLock(location))
    return;
  TRACE_EVENT1("view", "AutoLock Wait", "function", location.function_name());
  DomLock::GetInstance()->Acquire(location);
}

DomLock::AutoLock::~AutoLock() {
  DomLock::GetInstance()->Release(DomLock::GetInstance()->location());
  TRACE_EVENT_ASYNC_END0("view", "ViewLock", this);
}

//////////////////////////////////////////////////////////////////////
//
// DomLock::AutoTryLock
//
DomLock::AutoTryLock::AutoTryLock(const Location& location)
    : locked_(DomLock::GetInstance()->TryLock(location)) {
  if (!locked_) {
    TRACE_EVENT_INSTANT1("view", "ViewTryLock", TRACE_EVENT_SCOPE_THREAD,
                         "function", location.function_name());
    return;
  }
  TRACE_EVENT_ASYNC_BEGIN2("view", "ViewLock", this, "function",
                           location.function_name(), "type", "AutoTryLock");
}

DomLock::AutoTryLock::~AutoTryLock() {
  if (!locked_)
    return;
  DomLock::GetInstance()->Release(DomLock::GetInstance()->location());
  TRACE_EVENT_ASYNC_END0("view", "ViewLock", this);
}

//////////////////////////////////////////////////////////////////////
//
// DomLock::AutoUnlock
//
DomLock::AutoUnlock::AutoUnlock(const Location& location) {
  TRACE_EVENT_ASYNC_BEGIN2("view", "ViewLock", this, "function",
                           location.function_name(), "type", "AutoUnlock");
  DomLock::GetInstance()->Release(location);
}

DomLock::AutoUnlock::~AutoUnlock() {
  DomLock::GetInstance()->Acquire(DomLock::GetInstance()->location());
  TRACE_EVENT_ASYNC_END0("view", "ViewLock", this);
}

//////////////////////////////////////////////////////////////////////
//
// DomLock
//
DomLock::DomLock() : locked_(false) {}

DomLock::~DomLock() {}

const tracked_objects::Location& DomLock::location() const {
  return dom::Lock::instance()->location();
}

void DomLock::Acquire(const Location& location) {
  DCHECK(thread_checker_.CalledOnValidThread());
  dom::Lock::instance()->Acquire(location);
  locked_ = true;
}

void DomLock::AssertLocked(const Location& location) {
  if (locked_)
    return;
  LOG(ERROR) << "Assert locked at " << location.ToString() << ", but locked by "
             << this->location().ToString();
  NOTREACHED();
}

// static
DomLock* DomLock::GetInstance() {
  return base::Singleton<DomLock>::get();
}

void DomLock::Release(const Location& location) {
  DCHECK(thread_checker_.CalledOnValidThread());
  dom::Lock::instance()->Release(location);
  locked_ = false;
}

bool DomLock::TryLock(const Location& location) {
  DCHECK(thread_checker_.CalledOnValidThread());
  DCHECK(!locked_);
  locked_ = dom::Lock::instance()->TryLock(location);
  return locked_;
}

}  // namespace editor
