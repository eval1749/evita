// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/lock.h"

#include <ostream>

std::ostream& operator<<(std::ostream& ostream,
                         const tracked_objects::Location & location) {
  return ostream << location.ToString();
}

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// Lock::AutoLock
//
Lock::AutoLock::AutoLock(const Location& location)
    : base::AutoLock(*Lock::instance()->lock()) {
  DVLOG(1) << "Lock dom at " << location;
  Lock::instance()->location_ = location;
  Lock::instance()->locked_by_dom_ = true;;
}

Lock::AutoLock::~AutoLock() {
  Lock::instance()->locked_by_dom_ = false;
  DVLOG(1) << "Unlock dom at " << Lock::instance()->location_ ;
}

//////////////////////////////////////////////////////////////////////
//
// Lock::AutoTryLock
//
Lock::AutoTryLock::AutoTryLock(const Location& location)
    : locked_(Lock::instance()->lock()->Try()) {
  DVLOG(1) << "TryLock dom at " << location;
  Lock::instance()->location_ = location;
  if (locked_)
    Lock::instance()->locked_by_dom_ = true;
}

Lock::AutoTryLock::~AutoTryLock() {
  if (locked_) {
    Lock::instance()->locked_by_dom_ = false;
    Lock::instance()->lock()->Release();
  }
  DVLOG(1) << "Unlock dom at " << Lock::instance()->location_ ;
}

//////////////////////////////////////////////////////////////////////
//
// Lock::AutoUnlock
//
Lock::AutoUnlock::AutoUnlock(const Location& location)
    : base::AutoUnlock(*Lock::instance()->lock()) {
  DVLOG(1) << "Unlock dom at " << location;
  Lock::instance()->location_ = location;
  Lock::instance()->locked_by_dom_ = false;
}

Lock::AutoUnlock::~AutoUnlock() {
  DVLOG(1) << "Lock dom at " << Lock::instance()->location_ ;
  Lock::instance()->locked_by_dom_ = true;
}

//////////////////////////////////////////////////////////////////////
//
// Lock
//
Lock::Lock() : lock_(new base::Lock()), locked_by_dom_(false) {
}

Lock::~Lock() {
}

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
