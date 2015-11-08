// Copyright (c) 1996-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "evita/dom/lock.h"

#include "base/trace_event/trace_event.h"

std::ostream& operator<<(std::ostream& ostream,
                         const tracked_objects::Location& location) {
  return ostream << location.ToString();
}

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// Lock::AutoLock
//
Lock::AutoLock::AutoLock(const Location& location) {
  TRACE_EVENT_ASYNC_BEGIN2("script", "DomLock", this, "function",
                           location.function_name(), "type", "AutoLock");
  DVLOG(1) << "Lock dom at " << location;
  Lock::instance()->location_ = location;
  Lock::instance()->locked_by_dom_ = true;
  if (Lock::instance()->lock()->Try())
    return;
  TRACE_EVENT1("script", "AutoLock Wait", "function", location.function_name());
  Lock::instance()->lock()->Acquire();
}

Lock::AutoLock::~AutoLock() {
  Lock::instance()->locked_by_dom_ = false;
  Lock::instance()->lock()->Release();
  DVLOG(1) << "Unlock dom at " << Lock::instance()->location_;
  TRACE_EVENT_ASYNC_END0("script", "DomLock", this);
}

//////////////////////////////////////////////////////////////////////
//
// Lock::AutoTryLock
//
Lock::AutoTryLock::AutoTryLock(const Location& location)
    : locked_(Lock::instance()->lock()->Try()) {
  DVLOG(1) << "TryLock dom at " << location;
  Lock::instance()->location_ = location;
  if (!locked_) {
    TRACE_EVENT_INSTANT1("script", "DomTryLock", TRACE_EVENT_SCOPE_THREAD,
                         "function", location.function_name());
    return;
  }
  TRACE_EVENT_ASYNC_BEGIN2("script", "DomLock", this, "function",
                           location.function_name(), "type", "AutoTryLock");
  Lock::instance()->locked_by_dom_ = true;
}

Lock::AutoTryLock::~AutoTryLock() {
  if (locked_) {
    Lock::instance()->locked_by_dom_ = false;
    Lock::instance()->lock()->Release();
  }
  DVLOG(1) << "Unlock dom at " << Lock::instance()->location_;
  TRACE_EVENT_ASYNC_END0("script", "DomLock", this);
}

//////////////////////////////////////////////////////////////////////
//
// Lock::AutoUnlock
//
Lock::AutoUnlock::AutoUnlock(const Location& location)
    : base::AutoUnlock(*Lock::instance()->lock()) {
  TRACE_EVENT_ASYNC_BEGIN2("script", "DomLock", this, "function",
                           location.function_name(), "type", "AutoUnlock");
  DVLOG(1) << "Unlock dom at " << location;
  Lock::instance()->location_ = location;
  Lock::instance()->locked_by_dom_ = false;
}

Lock::AutoUnlock::~AutoUnlock() {
  DVLOG(1) << "Lock dom at " << Lock::instance()->location_;
  Lock::instance()->locked_by_dom_ = true;
  TRACE_EVENT_ASYNC_END0("script", "DomLock", this);
}

//////////////////////////////////////////////////////////////////////
//
// Lock
//
Lock::Lock() : lock_(new base::Lock()), locked_by_dom_(false) {}

Lock::~Lock() {}

void Lock::Acquire(const tracked_objects::Location& location) {
  location_ = location;
  lock_->Acquire();
}

void Lock::Release(const tracked_objects::Location& location) {
  location_ = location;
  lock_->Release();
}

bool Lock::TryLock(const tracked_objects::Location& location) {
  if (!lock_->Try())
    return false;
  location_ = location;
  return true;
}

}  // namespace dom
