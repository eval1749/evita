// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/editor/dom_lock.h"

#include "evita/dom/lock.h"
#include "evita/editor/application.h"

namespace editor {

//////////////////////////////////////////////////////////////////////
//
// DomLock::AutoLock
//
DomLock::AutoLock::AutoLock(const Location& location) {
  DomLock::instance()->Acquire(location);
}

DomLock::AutoLock::~AutoLock() {
  DomLock::instance()->Release(DomLock::instance()->location());
}

//////////////////////////////////////////////////////////////////////
//
// DomLock::AutoTryLock
//
DomLock::AutoTryLock::AutoTryLock(const Location& location)
    : locked_(DomLock::instance()->TryLock(location)) {
}

DomLock::AutoTryLock::~AutoTryLock() {
  if (!locked_)
    return;
  DomLock::instance()->Release(DomLock::instance()->location());
}

//////////////////////////////////////////////////////////////////////
//
// DomLock::AutoUnlock
//
DomLock::AutoUnlock::AutoUnlock(const Location& location) {
  DomLock::instance()->Release(location);
}

DomLock::AutoUnlock::~AutoUnlock() {
  DomLock::instance()->Acquire(DomLock::instance()->location());
}

//////////////////////////////////////////////////////////////////////
//
// DomLock
//
DomLock::DomLock() : locked_(false) {
}

DomLock::~DomLock() {
}

DomLock* DomLock::instance() {
  return Application::instance()->dom_lock();
}

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
  LOG(ERROR) << "Assert locked at " << location.ToString() <<
      ", but locked by " << this->location().ToString();
  NOTREACHED();
}

void DomLock::Release(const Location& location) {
  DCHECK(thread_checker_.CalledOnValidThread());
  dom::Lock::instance()->Release(location);
  locked_ = false;
}

bool DomLock::TryLock(const Location& location) {
  DCHECK(thread_checker_.CalledOnValidThread());
  if (!locked_)
    locked_ = dom::Lock::instance()->TryLock(location);
  return locked_;
}

}  // namespace editor
